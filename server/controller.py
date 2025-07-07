from flask import request, jsonify
import io
import requests

class Controller:
    def __init__(self, app_instance):
        self._app = app_instance
        self.__data = None

        self.__setup_routes()

    def __setup_routes(self):
        pass
        self._app.add_url_rule('/insert/data', view_func=self._insert_data, methods=['POST'])
        self._app.add_url_rule('/find/data', view_func=self._find_data, methods=['GET'])



    def _insert_data(self):
        try:
            data = request.get_json()
            self.__data = data.get('data')

            return jsonify({'message': 'Data saved successfully'}), 201
        except Exception as e:
            return str(e), 500
    
    def _find_data(self):
        try:
            if self.__data is not None:
                return jsonify(self.__data), 200
            else:
                return jsonify({'message': 'No data found.'}), 404
        except Exception as e:
            return str(e), 500

    def run(self):
        self._app.run(host='0.0.0.0')