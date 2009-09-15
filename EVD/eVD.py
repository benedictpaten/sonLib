#!/usr/bin/env python

import math

def calculateEVDProbFromScore(score, eVDNode, n):
    """Calculates the EVD probability of the score.
    """
    lambdaValue = float(eVDNode.find("lambda").attrib['value'])
    muValue = float(eVDNode.find("mu").attrib['value'])
    normValue = float(eVDNode.find("norm").attrib['value'])
    value = n/normValue
    
    pSGTX = 1.0 - math.exp(-math.exp(-lambdaValue * (score - muValue)))
    
    if value >= 5.0:
        return 1.0 - math.exp(-value*pSGTX)
    #This is an approximation for small values`
    return pSGTX

def calculateScoreFromEVDProb(prob, eVDNode, n):
    """Calculates the EVD probability of the score.
    """
    lambdaValue = float(eVDNode.find("lambda").attrib['value'])
    muValue = float(eVDNode.find("mu").attrib['value'])
    normValue = float(eVDNode.find("norm").attrib['value'])
    value = n/normValue
    
    if value >= 5.0:
        i = math.log(1.0 - prob)/-value
    else:
        i = prob
    
    j = math.log(-math.log(1.0 - i))/-lambdaValue + muValue
    return j

def main():
    pass

def _test():
    import doctest      
    return doctest.testmod()

if __name__ == '__main__':
    _test()
    main()
