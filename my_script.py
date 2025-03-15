import time
import requests
import hmac
import hashlib

# Введіть ваші API ключі
API_KEY = 'A0YyUPYTdTv7dPY9B71LRjMnnA3Kp26hvcJb8VlYbne5V7hz2fgpFNnbJmhK2OrG'
API_SECRET = '3PNJPuSOEha4SJ3M82xeCvfLvcDocvsc3f1DGucxQaV9PebN5MlZzHObQmNoNmHW'

def check_public_latency():
    """Вимірює затримку до публічного REST API Binance."""
    url = "https://api.binance.com/api/v3/ping"
    try:
        start_time = time.time()
        response = requests.get(url)
        end_time = time.time()
        
        if response.status_code == 200:
            latency = (end_time - start_time) * 1000
            print(f"Public API Latency: {latency:.2f} ms")
        else:
            print(f"Failed to reach public API. Status code: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"Error: {e}")

def check_private_latency():
    """Вимірює затримку до приватного REST API Binance з використанням API ключів."""
    url = "https://api.binance.com/api/v3/account"
    timestamp = int(time.time() * 1000)
    query_string = f"timestamp={timestamp}"
    signature = hmac.new(API_SECRET.encode(), query_string.encode(), hashlib.sha256).hexdigest()
    headers = {
        'X-MBX-APIKEY': API_KEY
    }
    params = {
        'timestamp': timestamp,
        'signature': signature
    }
    
    try:
        start_time = time.time()
        response = requests.get(url, headers=headers, params=params)
        end_time = time.time()
        
        if response.status_code == 200:
            latency = (end_time - start_time) * 1000
            print(f"Private API Latency: {latency:.2f} ms")
        else:
            print(f"Failed to reach private API. Status code: {response.status_code}")
            print("Response:", response.json())
    except requests.exceptions.RequestException as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    print("Measuring latency to Binance API...")
    while True:
        check_public_latency()
        check_private_latency()
        time.sleep(5)  # Оновлення кожні 5 секунд

