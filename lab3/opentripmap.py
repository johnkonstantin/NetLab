import requests
from geocoding import geocoding
from bs4 import BeautifulSoup

url = "https://api.opentripmap.com/0.1/ru"
API_KEY = "5ae2e3f221c38a28845f05b608ecf7df2342682bfe8bba4f040ebdff"


def get_point_properties(xid: str):
    return requests.get(url + "/places/xid/" + xid, params={'apikey': API_KEY}).json()


async def get_object_list(lat: float, lon: float, radius: float = 1000, limit: int = 1):
    query = {
        'lat': str(lat),
        'lon': str(lon),
        'radius': str(radius),
        'limit': str(limit),
        'lang': 'ru',
        'apikey': API_KEY
    }
    object_list_data = requests.get(url + "/places/radius", params=query).json()
    object_xid = [feature['properties']['xid'] for feature in object_list_data['features']]
    object_properties = [get_point_properties(xid) for xid in object_xid]
    object_description = [{
        'name': prop['name'],
        'otm': prop['otm'],
        'desc': prop['info']['descr'] if 'info' in prop.keys() else 'Отсутствует',
        'src': prop['info']['src'] if 'info' in prop.keys() else 'Отсутствует'
    } for prop in object_properties]
    print('\nКлассные места для тебя: ')
    for obj in object_description:
        if len(obj['name']) == 0:
            continue
        msg = obj['name']
        msg += '\nОписание: ' + BeautifulSoup(obj['desc'], features='html.parser').get_text() + '\n'
        msg += 'Официальный сайт: ' + obj['src'] + '\n'
        msg += 'Страница на opentripmap: ' + obj['otm'] + '\n'
        print(msg)


if __name__ == "__main__":
    answer = geocoding("Цветной проезд, Академгородок")
    point = answer['hits'][0]['point']
    places = get_object_list(point['lat'], point['lng'])
    print(places)
    # print("Достопримечательности  Академгородка:")
    # for feature in places['features']:
    #     print(feature['properties']['name'])
