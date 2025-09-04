from model import Model
from flask import request, jsonify

class Controller:
    def __init__(self, app_instance):
        self._app = app_instance
        self.__mq = None
        self.__ms = None
        self.__tgs = None

        self.__setup_routes()
        self.__db_model = Model()

    def __setup_routes(self):
        pass
        self._app.add_url_rule('/insert/data', view_func=self._insert_data, methods=['POST'])
        
    def _insert_data(self):
        try:
            data = request.get_json()
            self.__mq = data.get('mq')
            self.__ms = data.get('ms')
            self.__tgs = data.get('tgs')

            inserted_id = self.__db_model.insert_data(self.__mq, self.__ms, self.__tgs)

            return jsonify({'message': 'Data saved successfully', 'id': str(inserted_id)}), 201
        except Exception as e:
            return str(e), 500
# 
    def run(self):
        self._app.run(host='0.0.0.0')