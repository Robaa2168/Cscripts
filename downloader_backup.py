import requests
from bs4 import BeautifulSoup
import random
import os
import json
from urllib.parse import urlparse, urljoin
from queue import Queue
from charset_normalizer import from_bytes


# List of user agents for rotation
user_agents = [
    'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/58.0.3029.110 Safari/537.3',
]

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

def save_resource(url, domain):
    if is_spanish_url(url):
     print(f"Skipped Spanish resource: {url}")
     return
    # Define CDN patterns to skip
    cdn_patterns = ['cdn-', 'cloudfront.net', 'akamaihd.net', 'cdn.jsdelivr.net']

    # Check if the URL matches any CDN patterns
    if any(cdn_pattern in url for cdn_pattern in cdn_patterns):
        print(f"Skipped CDN resource: {url}")
        return

    if url in downloaded_resources:
        print(f"Already downloaded: {url}")
        return

    try:
        content = get_html(url)
        if content is not None:
            parsed_url = urlparse(url)
            file_path = os.path.join(domain, parsed_url.path.strip("/"))

            # Create directories as needed
            os.makedirs(os.path.dirname(file_path), exist_ok=True)

            # Write content based on its type
            if isinstance(content, bytes):
                with open(file_path, 'wb') as file:
                    file.write(content)
            else:  # It's a string (text)
                with open(file_path, 'w', encoding='utf-8') as file:
                    file.write(content)

            downloaded_resources.add(url)
            print(f"Saved resource: {file_path}")
            save_downloaded_resources() 
    except Exception as e:
        print(f"Error saving resource from {url}: {e}")



def save_html(domain, path, html_content):
    os.makedirs(domain, exist_ok=True)
    file_path = os.path.join(domain, path.strip("/").replace("/", "_") + ".html")
    with open(file_path, 'w', encoding='utf-8') as file:
        file.write(html_content)
    print(f"Saved HTML content to {file_path}")

def update_links_and_save_html(domain, path, html_content, base_url):
    soup = BeautifulSoup(html_content, 'html.parser')
    
    # Update href/src attributes for CSS, JS, images, and other resources
    for tag in soup.find_all(['link', 'script', 'img', 'a']):
        url_attr = 'href' if tag.name in ['link', 'a'] else 'src'
        if tag.get(url_attr):
            original_url = tag[url_attr]
            full_url = urljoin(base_url, original_url)
            tag[url_attr] = full_url

            # Check if the URL is for a resource that needs to be downloaded
            if any(ext in original_url for ext in ['.css', '.js', '.png', '.jpg', '.jpeg', '.gif', '.svg']):
                save_resource(full_url, domain)

    # Save the updated HTML content
    updated_html = soup.prettify(formatter="html5")
    save_html(domain, path, updated_html)


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
