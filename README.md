LOGIC

setup : turn on LM5 -> init PC&LM5 Serial -> wait LM5 Ready -> Check LM5's Network Property -> Connect Mqtt broker -> init Mqtt content

loop : set GPS On & Check -> Get GPS datas -> Publish GPS datas to Mqtt form : (bus_id, UTC, Lat, Lon)
