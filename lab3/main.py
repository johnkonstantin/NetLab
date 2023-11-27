from geocoding import geocoding
from weather import get_weather
from opentripmap import *
from bs4 import BeautifulSoup

if __name__ == '__main__':
    placeName = input("Введите название локации: ")
    answer = geocoding(placeName)
    count = len(answer['hits'])
    if count == 0:
        print("Ничего не найдено :(")
        exit(0)
    print("Найденные локации: ")
    for i, place in enumerate(answer['hits']):
        print((f"{i + 1}) {place['name']}, "
                f"{place['city'] if 'city' in place.keys() else '-'}, "
                f"{place['state'] if 'state' in place.keys() else '-'}, "
                f"{place['country'] if 'country' in place.keys() else '-'}"))
    while (True):
        index = int(input("Выберите номер локации: "))
        if index < 1 or index > count:
            print("Неверный ввод, повторите")
        else:
            break
    place = answer['hits'][index-1]
    print("Выбранная локация: ")
    print(f"{place['name']}, "
           f"{place['city'] if 'city' in place.keys() else '-'}, "
           f"{place['state'] if 'state' in place.keys() else '-'}, "
           f"{place['country'] if 'country' in place.keys() else '-'}")
    point = answer['hits'][index - 1]['point']
    lat, lon = point['lat'], point['lng']
    weather = get_weather(lat, lon)
    print("\nПогода: ")
    print("Температура: " + str(round(float(weather['main']['temp']) - 273.15, 2)))
    print("Ощущается как: " + str(round(float(weather['main']['feels_like']) - 273.15, 2)))
    radius = 0
    count_places = 0
    print("")
    while (True):
        radius = int(input("Введите радуис поиска классных мест в метрах: "))
        if radius <= 0:
            print("Радиус больше нуля!!!!!")
        else:
            break
    while (True):
        count_places = int(input("Введите количество классных мест: "))
        if radius <= 0:
            print("Количество больше нуля!!!!!")
        else:
            break
    object_list_data = get_object_list(lat, lon, radius, count_places)
    object_xid = [feature['properties']['xid'] for feature in object_list_data['features']]
    object_properties = [get_object_properties(xid) for xid in object_xid]
    object_description = [{
        'name': prop['name'],
        'otm': prop['otm'],
        'desc': prop['info']['descr'] if 'info' in prop.keys() else "Отсутствует",
        'src': prop['info']['src'] if 'info' in prop.keys() else "Отсутствует"
    } for prop in object_properties]
    print("\nКлассные места для тебя: ")
    for obj in object_description:
        if len(obj['name']) == 0:
            continue
        msg = obj['name']
        msg += "\nОписание: " + BeautifulSoup(obj['desc'], features="html.parser").get_text() + "\n"
        msg += "Официальный сайт: " + obj['src'] + "\n"
        msg += "Страница на opentripmap: " + obj['otm'] + "\n"
        print(msg)
