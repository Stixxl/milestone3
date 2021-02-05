import sys
import pickle
import mchmm as mc

def main():
    print('Hi from python forcaster')
    estimateCount = 0

    print('there are ' + str(len(sys.argv)) + ' arguments:')

    # hardcoded stuff at its best:
    if (len(sys.argv) != 3):
        print('forcaster expects different args')
        peopleCountArray = np.random.rand(45)
        mmFilePath = 'mmcm.pickle'
    else:
        peopleCountArray = sys.argv[1].split(',')
        peopleCountArray = list(map(int, peopleCountArray))

        print(peopleCountArray)

        mmFilePath = sys.argv[2]

    markovEstimateCount = getMarkovModelEstimate(peopleCountArray, mmFilePath)
    print('markovModel estimates ' + str(markovEstimateCount) + ' people')
    linregEstimateCount = getMarkovModelEstimate(linregFilePath)
    print("0:14:"+str(markovEstimateCount))

def getMarkovModelEstimate(peopleCountArray, modelPath):
    mod = pickle.load(open(modelPath, "rb"))
    ids, states = mod.simulate(5, start=peopleCountArray[0], seed = peopleCountArray[1:])
    return (ids[-1])

def getLinregEstimate(modelPath):
    mod = pickle.load(open(modelPath, "rb"))
    ids, states = mod.simulate(5, start=peopleCountArray[0], seed = peopleCountArray[1:])
    return (ids[-1])

main()