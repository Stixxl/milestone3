# Forecasting models derivation - on the example of SARIMAX
# References:
# http://www.statsmodels.org/dev/tsa.html
# http://www.statsmodels.org/dev/statespace.html
# http://www.statsmodels.org/dev/examples/notebooks/generated/statespace_sarimax_stata.html
from scipy.stats import norm
import mchmm as mc
import pickle
import sys
import os
import json
import http.client
from time import time
import pandas as pd
import numpy as np
from datetime import datetime
from sklearn.preprocessing import MinMaxScaler
from keras.models import Sequential
from keras.layers import LSTM, Dense
import tensorflow


def create_dataset(dataset, look_back=1):
    dataX, dataY = [], []
    for i in range(len(dataset) - look_back - 1):
        a = dataset[i:(i + look_back), 0]
        dataX.append(a)
        dataY.append(dataset[i + look_back, 0])
    return np.array(dataX), np.array(dataY)


CONSUMER_URL = 'iotplatform.caps.in.tum.de:443'
DEV_JWT = 'eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9' \
          '.eyJpYXQiOjE2MTIxMTg3NzgsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMjVfNjMifQ.gvarGnX729L0Ea42QnWQVTAqlzzL3eCx' \
          '-WxoOPlgwi8NBs0EBibDl9FHbFS_y0ZL89rEv9ApX9bOOJOfydes974COHwBE6Lq43KrhVxXMb5Pt3C5KaY_X' \
          '-Uo6zDrDtSeCq25JBFTbxzF89COlBlFo_tKyWNvMJOZCREfC8twZFyX6BR9bWxvgmAZs3g0cqCDn_GnTWP_L_5ADDX4nJ5Qr0zWb_EX5nQ8'\
          '3pCV2WLVpqX4t_zmvi0rcA5kK067cab9EP0cGQqdM6VTy9eM44jX4JI5I8WnZt7cF4TdNVcihkm4zVKkEqasT3C7V60HwhftSC0eg4VrUp' \
          '2v76GTVXez2642_yf2q0kYuwYSyEXncTKgpQSV_3Z76rMRlNepuZ8dp-gXm32-SSJC6iJ-ZnFyvKWGRoT3KBGu9AJvM34F11zYPlqWM' \
          'vg93wt9-iah1q_Pe3hIGYXWN52FPGcMgwlc1vFNDBDwrMkif7p85SdklMZeRAVvlc4sSWUu3V4-O7IWR7OhKYrDs5QfwhBHcKx79Spk' \
          'qTVIvEX0pt0776XmC2Fnodt9BOrn937Y12zn3dQjPcpgZFkkLCGTHE8d7yFrHCE8BzDRPE2Pynvkkpi6-dBU_HCZmqqKSVDFj-iaAARxu1' \
          'D_55rUXNafF-SYvLnQ9XDhqRclvftfy7-1TGUEVU8 '

if len(sys.argv) == 5:
    sensorID = sys.argv[1]
    modelFilepath = sys.argv[2]
    timeHorizonWeeks = int(sys.argv[3]) # 1 - for 1 week, 0 for all the data
    batchSize = int(sys.argv[4])
    searchPath = '/api/consumers/consume/' + sensorID + '/_search?' #base path - search all
    countPath = '/api/consumers/consume/' + sensorID + '/_count?' #base path - count all

    retrievedData = pd.DataFrame(columns=['t', 'count'])

    # I: https://www.elastic.co/guide/en/elasticsearch/reference/current/query-dsl-range-query.html
    if timeHorizonWeeks != 0:
        ts = time()
        curTs = int(ts - (ts % 60))
        weekAgoTs = int(curTs - timeHorizonWeeks * 7 * 24 * 60 * 60)
        sQuery = 'q=timestamp:[' + str(weekAgoTs * 1000) + '%20TO%20' + str(curTs * 1000) + ']'
        searchPath = searchPath + sQuery
        countPath = countPath + sQuery

    # Get the data for the last week
    consumerConn = http.client.HTTPSConnection(CONSUMER_URL)
    consumerConn.connect()

    headers = {  "Content-Type": "application/x-www-form-urlencoded", "Authorization": "Bearer " + DEV_JWT }
    consumerConn.request('GET', countPath, '', headers)
    iotPlResp = consumerConn.getresponse()
    rawCountData = iotPlResp.read()
    respCountData = json.loads(rawCountData)
    countLeft = int(respCountData["count"])
    begIndex = 0

    while countLeft > 0:
        # Slicing our requests
        searchPath = searchPath + '&from=' + str(begIndex) + '&size=' + str(batchSize)

        consumerConn.request('GET', searchPath, '', headers)
        # I: https://docs.python.org/3/library/http.client.html#httpresponse-objects
        iotPlResp = consumerConn.getresponse()
        rawData = iotPlResp.read()
        respData = json.loads(rawData)
        observationsArray = respData["hits"]["hits"]

        for observation in observationsArray:
            # Important to convert to seconds, datetime cannot handle ms
            # TODO: consider rounding to minutes
            timestamp_s = int(observation['_source']['timestamp'] / 1000)
            count = int(observation['_source']['value'])
            cur_date = datetime.fromtimestamp(timestamp_s)
            df_row = pd.DataFrame([[cur_date, count]], columns=['t', 'count'])
            retrievedData = retrievedData.append(df_row)

        begIndex = begIndex + batchSize
        countLeft = countLeft - batchSize

    # print(retrievedData)
    consumerConn.close()
    retrievedData.index = retrievedData.t

    scaler = MinMaxScaler(feature_range=(0, 1))
    dataset = scaler.fit_transform(np.array(retrievedData['count'].astype(float)).reshape(-1, 1))
    look_back = 20
    trainX, trainY = create_dataset(dataset, look_back)
    trainX = np.reshape(trainX, (trainX.shape[0], 1, trainX.shape[1]))

    print('Creating new model.')
    model = Sequential()
    model.add(LSTM(4, input_shape=(1, look_back)))
    model.add(Dense(1))
    model.compile(loss='mean_squared_error', optimizer='adam')
    model.fit(trainX, trainY, epochs=100, batch_size=1, verbose=2)
    model.save(modelFilepath)

else:
    print("Not enough arguments. The call should be: python3 arimamodelderivator.py <sensor id> <model file name> <number of weeks|0> <batch size>")
