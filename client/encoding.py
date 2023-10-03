def encoder(data):
    dataStr = "" + str(data['type'])
    if(data['data']):
        dataStr += ";" + str(data['data'][0]) + ";" + str(data['data'][1])
    print(dataStr)
    return dataStr.encode('utf-8')

def decoder(data):
    dataDecoded = {}
    data = data.decode('utf-8').split(';')
    dataDecoded['type'] = data[0]
    if(len(data) > 1):
        dataDecoded['data'] = [int(data[1]), int(data[2])]
    else:
        dataDecoded['data'] = None
    return dataDecoded