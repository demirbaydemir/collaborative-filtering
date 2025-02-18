#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <sstream>
#include <queue>
#include <iomanip>
using namespace std;

unordered_map<int, unordered_map<int, double>> userRatings;      // user -> {movieID -> rating}
unordered_map<int, unordered_map<int, double>> userSimilarities; // user -> {userID -> similarity}
unordered_map<int, double> userMeans;

double weightedSum;
double similaritySum;
int otherUsersID;
double e_rating;
double e_totalSum;
double userMean;

double estimateRating(int &UserID, int &movieID, double globalMean)
{

    if (userMeans.find(UserID) == userMeans.end())
    {
        // Calculate and store user mean if not already computed
        e_totalSum = 0;
        for (const auto &entry : userRatings[UserID])
        {
            e_totalSum += entry.second;
        }
        userMeans[UserID] = e_totalSum / userRatings[UserID].size();
    }
    userMean = userMeans[UserID];

    priority_queue<pair<double, int>> topUsers;

    for (const auto &[otherUserID, similarity] : userSimilarities[UserID])
    {
        if (userRatings.count(otherUserID) && userRatings[otherUserID].count(movieID))
        {
            topUsers.emplace(similarity, otherUserID);
        }
    }

    weightedSum = 0;
    similaritySum = 0;
    int count = 0;
    double otherUserRating;

    while (!topUsers.empty() && count < 50)
    {
        auto [similarity, otherUserID] = topUsers.top();
        topUsers.pop();

        otherUserRating = userRatings[otherUserID][movieID];
        weightedSum += similarity * otherUserRating;
        similaritySum += similarity;
        count++;
    }

    if (similaritySum == 0)
        return userMean; // No similar users found

    return userMean + 0.75 * (weightedSum / similaritySum - globalMean);
}

// Calc Similarity
double sumSquaredDiff;
int c_count;
int c_movieID;
double mainRating;
double secondaryRating;

void calcMSDSimilarity(int &mainUser, int &secondaryUser, unordered_map<int, double> &mainUserRatings, unordered_map<int, double> &secondaryUserRatings)
{
    sumSquaredDiff = 0;
    c_count = 0;
    mainUserRatings = userRatings[mainUser];
    secondaryUserRatings = userRatings[secondaryUser];

    for (const auto &movieRatingPair : mainUserRatings)
    {
        c_movieID = movieRatingPair.first;

        if (secondaryUserRatings.find(c_movieID) != secondaryUserRatings.end())
        {
            mainRating = movieRatingPair.second;
            secondaryRating = secondaryUserRatings[c_movieID];
            sumSquaredDiff += pow(mainRating - secondaryRating, 2);
            c_count++;
        }
    }

    if (c_count < 50)
    {
        userSimilarities[mainUser][secondaryUser] = 0; // Low common ratings
        return;
    }

    // Transform MSD into similarity
    userSimilarities[mainUser][secondaryUser] = 1 / (1 + sumSquaredDiff / c_count);
}

// Main
int otherUsers;
int userID, movieID;
double m_rating;

int main()
{
    double sumRatings = 0;
    int count = 0;

    ios::sync_with_stdio(false);
    cin.tie(0);

    string line;
    bool isTest = false;
    stringstream ss;

    while (getline(cin, line))
    {
        if (line == "train dataset")
        {
            isTest = false;
            continue;
        }
        else if (line == "test dataset")
        {
            isTest = true;
            continue;
        }

        ss.str(line);

        if (!isTest)
        {
            ss >> userID >> movieID >> m_rating;
            sumRatings += m_rating;
            count++;
            userRatings[userID][movieID] = m_rating;
        }
        else
        {
            ss >> userID >> movieID;

            if (userSimilarities.find(userID) == userSimilarities.end() || userSimilarities[userID].empty())
            {
                // If not, calculate similarities for this user
                for (const auto &pairs : userRatings)
                {
                    otherUsers = pairs.first;

                    // Skip if comparing the userID with itself
                    if (userID == otherUsers)
                        continue;

                    // Calculate similarity and store it
                    calcMSDSimilarity(userID, otherUsers, userSimilarities[userID], userSimilarities[otherUsers]);
                }
            }
            cout << estimateRating(userID, movieID, sumRatings / count) << endl;
        }
        ss.clear();
    }
    return 0;
}
