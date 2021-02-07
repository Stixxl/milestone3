import sys
import pickle
import numpy as np
import mchmm as mc
import requests
import time
import zipfile
import keras
import datetime
import math

from sklearn.metrics import mean_squared_error
from datetime import date

url = "http://35.242.193.115:8080/"


def main():
    print('Hi from python forcaster')
    print('check for latest models')
    updateModels()

    estimateCount = 0

    print('there are ' + str(len(sys.argv)) + ' arguments:')

    # hardcoded stuff at its best:
    if (len(sys.argv) != 2):
        print('forcaster expects different args')
        peopleCountArray = np.random.randint(30, size=45)
        #peopleCountArray = np.zeros(45)
        peopleCountArray = peopleCountArray.astype(int)
    else:
        peopleCountArray = sys.argv[1].split(',')
        peopleCountArray = list(map(int, peopleCountArray))
        print(peopleCountArray)

    mec = getMarkovModelEstimate(peopleCountArray)
    print('markovModel estimates ' + str(mec) + ' people')
    lec = getLinregEstimate()
    print('linregModel estimates ' + str(lec) + ' people')
    aec = getArimaEstimate()
    print('arimaModel estimates ' + str(aec) + ' people')
    rec = getRNNEstimate(peopleCountArray)
    print('rnnModel estimates ' + str(rec) + ' people')

    lastOnlineValues = get_latestEstimates()
    lastOnlineValues = np.roll(lastOnlineValues,1)
    lastOnlineValues[:, 0] = np.array([mec, lec, aec, rec, 0]).T
    lastOnlineValues[4, 45] = peopleCountArray[0]
    bestGuess = TakeEstimateWithHighestAccuracy(lastOnlineValues)
    set_latestEstimates(lastOnlineValues)

    print("0:14:" + str(bestGuess))


def getMarkovModelEstimate(peopleCountArray):

    start = peopleCountArray[0].item()
    mod = pickle.load(open("markov.pickle", "rb"))
    ids, states = mod.simulate(5, start=start)
    return (ids[-1])


def getLinregEstimate():
    mod = pickle.load(open("linreg.pickle", "rb"))
    ts = np.full([1, 1], int(time.time()) + (60 * 15))
    # to the future!
    # add 15 minutes to it
    return int(mod.predict(ts.reshape(1, -1)))

def getArimaEstimate():
    mod = pickle.load(open("arima.pickle", "rb"))
    predictions = mod.forecast(45)
    return int(predictions.values[-1])

def getRNNEstimate(peopleCountArray):
    look_back = 20

    mod = keras.models.load_model(f"rnnmodel")

    #prediction_list = peopleCountArray
    prediction_list = np.empty(0)

    for _ in range(len(peopleCountArray)):
        x = peopleCountArray[-look_back:]
        x = x.reshape((1, 1, look_back))
        out = mod.predict(x)[0][0]
        prediction_list = np.append(prediction_list, out)
    #prediction_list = prediction_list[look_back:]

    return prediction_list[-1].astype(int)

def TakeEstimateWithHighestAccuracy(lastOnlineValues):
    #1.mm 2.linreg 3.arima 4.rnn
    evalArray = lastOnlineValues[:, 45:]

    rmseArray = np.zeros(4)

    #taking RMSE as value to choose model from.
    rmseArray[0] = math.sqrt(mean_squared_error(evalArray[0, :], evalArray[4, :]))
    rmseArray[1] = math.sqrt(mean_squared_error(evalArray[1, :], evalArray[4, :]))
    rmseArray[2] = math.sqrt(mean_squared_error(evalArray[2, :], evalArray[4, :]))
    rmseArray[3] = math.sqrt(mean_squared_error(evalArray[3, :], evalArray[4, :]))

    print("rmse values")
    print("markov: " + str(rmseArray[0]))
    print("linreg: " + str(rmseArray[1]))
    print("arima: " + str(rmseArray[2]))
    print("rnn: " + str(rmseArray[3]))

    #returning estimate for model that was best over the last 45 estimations
    return int(lastOnlineValues[np.argmax(rmseArray), 0])

def getFileFromServer(controllerName, fileName):
    r = requests.get(url + controllerName)
    if r.ok:
        with open(fileName, 'wb') as f:
            f.write(r.content)


#State that holds the latest estimated values
def get_latestEstimates():
    try:
        with open('latestEstimates', 'rb') as f:
            status = pickle.load(f)
            return status
    except:
        return np.zeros((5, 90))

def set_latestEstimates(estimates):
    with open('latestEstimates', 'wb') as f:
        pickle.dump(estimates, f)

#Flag that tells when the last model update was done
def get_latestModelUpdate():
    try:
        with open('latestModelUpdate', 'rb') as f:
            status = pickle.load(f)
            return status
    except:
        return datetime.datetime.now() - datetime.timedelta(8)

def set_latestModelUpdate(date):
    with open('latestModelUpdate', 'wb') as f:
        pickle.dump(date, f)

def updateModels():
    latestUpdateDate = get_latestModelUpdate()
    delta = datetime.datetime.now().date() - latestUpdateDate
    print("last update was " + str(delta.days) + " days ago")
    diff = 7
    if int(delta.days) >= diff:
        set_latestModelUpdate(date.today())
        print("update was longer than " + str(diff) + " days ago, updating models...")
        getFileFromServer("markov", "markov.pickle")
        getFileFromServer("linreg", "linreg.pickle")
        getFileFromServer("rnn", "rnn.zip")
        getFileFromServer("arima", "arima.pickle")
        # Random seconds to make sure download is done.. .
        time.sleep(30)

        with zipfile.ZipFile("rnn.zip", 'r') as zip_ref:
            zip_ref.extractall()

        print("all done")




main()
