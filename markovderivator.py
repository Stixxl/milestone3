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

from time import time
import pandas as pd
from datetime import datetime
import requests

CONSUMER_URL = 'https://iotplatform.caps.in.tum.de'
DEV_JWT = 'eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJpYXQiOjE2MTI0NDk3NzQsImlzcyI6ImlvdHBsYXRmb3JtIiwic3ViIjoiMjVfNjMifQ.vgNjk3NN5xvDAKIvrZPClLg-wscqORVrgeB7cHYbzY4VUnPlYGILHWlPpY44w5A1GyyC_6vPWtQU1c9fAdnkbCOv9KnU2_dOUjB9InUzZRhVn8hGtT9K1oojszlO4gQfVa2hT8CiAClcYDTsNnBetqgb95-k6jepR8iEnd4EVQAvjDwDnI_-VD9cAZJW9kRLF-49zJrCX4vUPrNNQr9Qt2CD35HM0Dq1Gb272tDEz7lgTwe1U1xHx0VA9Fdjn6Hp0XSzePv4Le4z_-FivP1Dr1sFDf4U7jgrWbqUrE_oznl2Hlc9udGx_vFuSRXiiGb39DsBHDqBCeuMP4WkOKE7RQWyPx9UHvJgqWTC42tBPo0oc_KPfwgcE6YUXXYjX_ZMiblGqU0XPXa8mSkhKNxMEnMpoKmNNFjygQeLkuZ5eT6Dbc8S-KmrJ4BzKZUqg9zslzJIQcweLXsJpL9-y63NivdtT-zzR5PL3ZORc-Ok9DoZnFYvg7yzYx6PXydIWEMYpuTx1o0107C91K5Kf1Mli_Hv2YxQzulsAOmsff9CUSYfpxiRzTEsC7G3lHmRYTzi6k8LZRnFJKrJ_2l2PUUs2UYhyqezSQsfP_LmG4zg4iTsOQxb-GUYENeB7W5sssU3UJZVhvoAQWh7t8sjvtmQql2lx41eVc5lwM8RAHhGSvw'

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



    headers = {  "Content-Type": "application/x-www-form-urlencoded", "Authorization": "Bearer " + DEV_JWT }
    # Get the data for the last week
    response = requests.get(CONSUMER_URL + countPath, headers=headers, verify=False)
    respCountData = response.json()
    countLeft = int(respCountData["count"])
    begIndex = 0

    while countLeft > 0:
        # Slicing our requests
        searchPath = searchPath + '&from=' + str(begIndex) + '&size=' + str(batchSize)

        response = requests.get(CONSUMER_URL + searchPath, headers=headers, verify=False)
        # I: https://docs.python.org/3/library/http.client.html#httpresponse-objects
        respData = response.json()
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

    print(retrievedData)
    retrievedData.index = retrievedData.t

    # Create the model from scratch
    mod = mc.MarkovChain().from_data(retrievedData['count'])
    print(mod.expected_matrix)
    pickle.dump(mod, open(modelFilepath, 'wb'))

else:
    print("Not enough arguments. The call should be: python3 markovderivator.py <sensor id> <model file name> <number of weeks|0> <batch size>")
