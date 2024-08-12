import requests
from bs4 import BeautifulSoup
import random
import os
import json
import hashlib
from urllib.parse import urlparse, urljoin
from queue import Queue
from charset_normalizer import from_bytes
import mimetypes
import logging

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')


# List of user agents for rotation
user_agents = [
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3',
]

base_url = 'https://www.anglogoldashanti.com/'
domain = urlparse(base_url).netloc

# Load or initialize the list of already scraped URLs
try:
    with open('scraped_urls.json', 'r') as file:
        scraped_urls = set(json.load(file))
except (FileNotFoundError, json.JSONDecodeError):
    scraped_urls = set()

# Initialize a set for already downloaded resources
try:
    with open('downloaded_resources.json', 'r') as file:
        downloaded_resources = set(json.load(file))
except (FileNotFoundError, json.JSONDecodeError, Exception):
    downloaded_resources = set()


def is_spanish_url(url):
    """Check if the URL or filename is likely to point to Spanish content."""
    spanish_indicators = ['es_', 'espanol', 'spanish']
    parsed_url = urlparse(url)
    path_segments = parsed_url.path.split('/')
    return any(segment.startswith(tuple(spanish_indicators)) for segment in path_segments)


def save_downloaded_resources():
    try:
        if downloaded_resources:
            with open('downloaded_resources.json', 'w') as file:
                json.dump(list(downloaded_resources), file)
            print("Downloaded resources saved successfully.")
        else:
            print("No downloaded resources to save.")
    except Exception as e:
        print(f"Error saving downloaded resources: {e}")


def get_html(url):
    try:
        headers = {'User-Agent': random.choice(user_agents)}
        response = requests.get(url, headers=headers, timeout=10)
        response.raise_for_status()

        # Try detecting the encoding for text-based content
        match = from_bytes(response.content).best()
        if match:
            # If an encoding is detected successfully, use it to decode the content
            return match.output()
        else:
            # Fallback: if encoding couldn't be detected, try with UTF-8, then 'latin-1'
            try:
                return response.content.decode('utf-8')
            except UnicodeDecodeError:
                return response.content.decode('latin-1')

    except requests.exceptions.RequestException as e:
        print(f"Error fetching {url}: {e}")
        return None

def get_content_type(url):
    """
    Guess content type based on the URL
    """
    type, _ = mimetypes.guess_type(url)
    return type

def save_resource(url, domain, base_url):
    if is_spanish_url(url) or url in downloaded_resources:
        print(f"Resource already processed or is a Spanish URL: {url}")
        return

    parsed_url = urlparse(url)
    if parsed_url.netloc != urlparse(base_url).netloc:
        print(f"External resource skipped: {url}")
        return

    # Generate the file path
    file_path = os.path.join(domain, parsed_url.path.lstrip('/'))
    if parsed_url.query:
        query_hash = hashlib.md5(parsed_url.query.encode('utf-8')).hexdigest()
        file_path += '_' + query_hash

    # Check and adjust the file extension
    mime_type, _ = mimetypes.guess_type(url)
    file_extension = mimetypes.guess_extension(mime_type) if mime_type else ''
    
    # Correctly handle PDF files to prevent appending '.html'
    if mime_type == 'application/pdf':
        if not file_path.endswith('.pdf'):
            file_path += '.pdf'  # Ensure the PDF extension is correctly applied
    elif not os.path.splitext(file_path)[1]:  # For other files without an extension
        file_path += file_extension or '.html'  # Default to '.html' if no extension is determined

    # Ensure the directory structure exists
    os.makedirs(os.path.dirname(file_path), exist_ok=True)

    try:
        headers = {'User-Agent': random.choice(user_agents)}
        response = requests.get(url, headers=headers, timeout=10)
        response.raise_for_status()

        with open(file_path, 'wb') as file:
            file.write(response.content)

        downloaded_resources.add(url)
        print(f"Saved resource: {file_path}")
    except Exception as e:
        print(f"Error saving resource {url}: {e}")

    save_downloaded_resources()



def make_relative_url(full_url, base_url):
    """Convert an absolute URL to a relative one if it starts with the base URL."""
    parsed_full_url = urlparse(full_url)
    parsed_base_url = urlparse(base_url)
    if parsed_full_url.netloc == parsed_base_url.netloc:
        # Return only the path, query, and fragment, making it relative
        return parsed_full_url.path + ('?' + parsed_full_url.query if parsed_full_url.query else '') + ('#' + parsed_full_url.fragment if parsed_full_url.fragment else '')
    else:
        # Return the original URL if it's not internal
        return full_url

def update_links_and_save_html(domain, path, html_content, base_url):
    try:
        soup = BeautifulSoup(html_content, 'html.parser')
    except Exception as e:
        logging.error(f"Error parsing HTML content from {base_url}{path}: {e}")
        return  # Gracefully skip processing this content

    # Process and update links within the HTML content
    for tag in soup.find_all(['a', 'link'], href=True):  # Including 'link' for stylesheets
        original_url = tag['href']
        full_url = urljoin(base_url, original_url)
        parsed_full_url = urlparse(full_url)

        # Convert to relative URL for internal links pointing to HTML content
        if parsed_full_url.path.endswith('.html') or parsed_full_url.path.endswith('/'):
            tag['href'] = make_relative_url(full_url, base_url)
        # Skip modifying URLs for non-HTML content like PDFs
        elif not parsed_full_url.path.endswith('.pdf'):
            tag['href'] = full_url  # This maintains the original absolute URL

    # Save the updated HTML content
    updated_html = soup.prettify(formatter="html5")
    save_html(domain, path, updated_html)

def save_html(domain, path, html_content):
    # Correct handling to ensure 'index.html' is used when no specific filename is provided
    directory_path = os.path.join(domain, os.path.dirname(path.lstrip('/')))
    os.makedirs(directory_path, exist_ok=True)
    file_name = os.path.basename(path)
    if not file_name:
        file_name = 'index.html'  # Use 'index.html' for directories
    elif not file_name.endswith('.html'):
        file_name += '.html'  # Append '.html' if missing
    file_path = os.path.join(directory_path, file_name)
    
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(html_content)
    logging.info(f"Saved HTML content to {file_path}")

def find_links(url, html_content):
    soup = BeautifulSoup(html_content, 'html.parser')
    for link in soup.find_all('a', href=True):
        full_link = urljoin(url, link['href'])
        if urlparse(full_link).netloc == urlparse(url).netloc:
            yield full_link

def scrape_url(start_url, max_depth=5):
    queue = Queue()
    queue.put((start_url, 0))

    while not queue.empty():
        current_url, depth = queue.get()
        if depth > max_depth or is_spanish_url(current_url):
            continue

        if current_url in scraped_urls:
            continue

        html = get_html(current_url)
        if html:
            parsed_url = urlparse(current_url)
            domain = parsed_url.netloc
            path = parsed_url.path

            try:
                decoded_html = html.decode('utf-8')
            except AttributeError:
                decoded_html = html

            update_links_and_save_html(domain, path, decoded_html, current_url)
            scraped_urls.add(current_url)

            if depth < max_depth:
                for link in find_links(current_url, decoded_html):
                    queue.put((link, depth + 1))

            with open('scraped_urls.json', 'w') as file:
                json.dump(list(scraped_urls), file)


def main():
    start_url = input("Enter the URL to start scraping: ")
    scrape_url(start_url)

if __name__ == "__main__":
    main()
