def isPat(hand):
    h = [card-1 for card in hand]
    if len(set(card//13 for card in h)) == 1:
        return 1   #flush
    ranks = [card % 13 for card in h]
    m = ranks.count(min(ranks))
    M = ranks.count(max(ranks))
    if {m,M} == {2,3}:
        return 1  # full house
    if len(set(ranks)) != 5:
        return 0
    ranks.sort()
    if ranks[4]-ranks[0] == 4:
        return 1
    return ranks == [0,9,10,11,12,]

def score(problem, solution):
    cards = 0
    goodPats = 0
    badPats = 0

    cardScore = list(range(26))
    goodPatScore = [0,15,30,45,60,75]
    badPatScore = 6*[0]
    for hand in range(0, 25, 5):
        good = 1
        for j in range(hand, hand+5):
            if problem[solution[j]]:
                cards += 1
            else:
                good = 0
            problem[solution[j]] = 0  #can't use same card twice
        if isPat(problem[hand:hand+5]):
            if good:
                goodPats += 1
            else:
                badPats += 1
        return cardScore[cards] + goodPatScore[goodPats] + badPatScore[badPats]