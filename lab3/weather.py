import requests
from geocoding import geocoding


url = "https://api.openweathermap.org/data/2.5/weather"
API_KEY = "6a04ca1a73622b434f11a249d5a7d1d1"


def get_weather(lat: float, lon: float) -> dict:
    query = {
        "lat": str(lat),
        "lon": str(lon),
        "appid": API_KEY
    }
    return requests.get(url, params=query).json()


if __name__ == "__main__":
    answer = geocoding("Цветной проезд, Академгородок")
    point = answer['hits'][0]['point']
    weather = get_weather(point['lat'], point['lng'])
    print(weather)
