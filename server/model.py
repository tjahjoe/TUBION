from pymongo import MongoClient
from pymongo.server_api import ServerApi
from datetime import datetime
import os

class Model():
    def __init__(self):
        self.__client = MongoClient(
            os.environ.get('MONGODB_API'), server_api=ServerApi('1'))
        try:
            self.__database = self.__client.get_database('tubion')
            self.__collection()
        except Exception as e:
            print(f'Error: {e}')

    def __collection(self):
        self.__collection_data = self.__database.get_collection('tubion')
    
    def insert_data(self, mq, ms, tgs):
        now = datetime.now()
        year = now.year
        month = now.month
        day = now.day
        hour = now.hour
        minute = now.minute

        data = {
            'year' : year,
            'month' : month,
            'day' : day,
            'hour' : hour,
            'minute' : minute,
            'data' : {
                'mq': mq,
                'ms': ms,
                'tgs': tgs,
                }
            }
        
        insert_result = self.__collection_data.insert_one(data)
        return insert_result.inserted_id


