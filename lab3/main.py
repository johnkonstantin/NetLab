from geocoding import geocoding
from weather import get_weather
from opentripmap import *
import asyncio


async def main():
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
    place = answer['hits'][index - 1]
    print("Выбранная локация: ")
    print(f"{place['name']}, "
          f"{place['city'] if 'city' in place.keys() else '-'}, "
          f"{place['state'] if 'state' in place.keys() else '-'}, "
          f"{place['country'] if 'country' in place.keys() else '-'}")
    point = answer['hits'][index - 1]['point']
    lat, lon = point['lat'], point['lng']
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
    weather = get_weather(lat, lon)

    await weather
    await object_list_data


if __name__ == '__main__':
    asyncio.run(main())
