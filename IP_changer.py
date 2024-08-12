from selenium import webdriver
from selenium.webdriver.common.by import By
import time

class IPChanger:
    def __init__(self):
        self.ips = [
            '8.8.8.8',         # Google Public DNS
            '208.67.222.222',  # OpenDNS
            '4.2.2.2',         # Level3 DNS
            '151.101.1.69',    # GitHub
            '151.101.65.194',  # GitHub
            '185.199.108.153', # GitHub
            '87.250.250.242',  # Yandex
            '93.184.216.34',   # Example.com
            '104.16.248.249',  # Cloudflare
            '13.107.42.13'     # Microsoft
        ]

        self.index = 0

    def get_next_ip(self):
        ip = self.ips[self.index]
        self.index = (self.index + 1) % len(self.ips)
        return ip

ip_changer = IPChanger()

chrome_options = webdriver.ChromeOptions()

for _ in range(10):
    chrome_options = webdriver.ChromeOptions()
    proxy = ip_changer.get_next_ip() + ":8080"  # Assuming the proxy is on port 8080
    chrome_options.add_argument('--proxy-server=%s' % proxy)

    driver = webdriver.Chrome(options=chrome_options)

    # Open the specified URL
    driver.get("https://strawpoll.com/eNg69WGV4nA")
    
    # Wait for the page to load properly
    time.sleep(5)  # you might need to adjust this wait time
    
    # Click on the option using the given XPath
    option = driver.find_element(By.XPATH, '//*[@id="option-eNg69WGV4nA-xVg7652renr"]')
    option.click()
    
    # Click on the submit button using the given XPath
    submit_button = driver.find_element(By.XPATH, '//*[@id="strawpoll_box_eNg69WGV4nA"]/div[2]/div[1]/form/div[9]/div[1]/div/button')
    submit_button.click()

   
