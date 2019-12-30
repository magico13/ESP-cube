# python module to make interfacing with the cube simpler
import requests
import json

class Animation(object):
    def __init__(self):
        self.animation_type = "None"
    
    def to_json(self):
        return f'{{"animation":{self.animation_type}}}'

class Blink(Animation):
    def __init__(self, count=1, wait=100, red1=0, green1=0, blue1=255, red2=0, green2=0, blue2=0):
        self.Count = count
        self.Wait = wait
        self.Red1 = red1
        self.Green1 = green1
        self.Blue1 = blue1
        self.Red2 = red2
        self.Green2 = green2
        self.Blue2 = blue2
        self.animation_type = "blink"

    def to_json(self):
        data = {
            "animation": "blink",
            "count": self.Count,
            "wait": self.Wait,
            "color": [
                self.Red1,
                self.Green1,
                self.Blue1
            ],
            "color2": [
                self.Red2,
                self.Green2,
                self.Blue2
            ]
        }
        return json.dumps(data)

class Breathe(Animation):
    def __init__(self, count=1, length=1000, red=0, green=0, blue=255):
        self.Count = count
        self.Length = length
        self.Red = red
        self.Green = green
        self.Blue = blue
        self.animation_type = "breathe"

    def to_json(self):
        data = {
            "animation": "breathe",
            "count": self.Count,
            "length": self.Length,
            "color": [
                self.Red,
                self.Green,
                self.Blue
            ]
        }
        return json.dumps(data)

class Cube():
    def __init__(self, url):
        self.BASEURL = url

    def get_color(self):
        code, json = self.get('/color')
        if code == 200: return json['red'], json['green'], json['blue']
        return 0, 0, 0

    def set_color(self, red, green, blue):
        data = f'{{"red":{red}, "green":{green}, "blue":{blue}}}'
        self.post('/color', data)

    def animate(self, animation):
        data = animation.to_json()
        self.post('/animate', data)

    def set_tap(self, animation):
        data = animation.to_json()
        self.post('/tap', data)

    def get(self, path):
        r = requests.get(self.BASEURL+path)
        if r.text:
            return r.status_code, r.json()
        return r.status_code, ''
    
    def post(self, path, data):
        r = requests.post(self.BASEURL+path, data=data)
        if r.text:
            return r.status_code, r.json()
        return r.status_code, ''