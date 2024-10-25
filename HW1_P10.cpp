#include <bits/stdc++.h>
#include <sys/time.h>
#define N 200
using namespace std;

typedef vector<pair<int, double>> vp;

double innerProduct(vp a, vp b) {
    unordered_map<int, double> m;

    for (auto i : b)
	{
        m[i.first] = i.second;
    }

    double result = 0.0;

    for (auto i : a)
	{
        result += i.second * m[i.first];
    }
    return result;
}

vp add(vp a, vp b)
{
	vp res;
	auto i = a.begin(), j = b.begin();
	while(i != a.end() && j != b.end())
	{
		if((*i).first < (*j).first)
			res.push_back((*i++));
		else if((*i).first > (*j).first)
			res.push_back((*j++));
		else
		{
			res.push_back({(*i).first, (*i).second + (*j).second});
			i++;j++;
		}
	}
	while(i != a.end()) res.push_back(*i++);
	while(j != b.end()) res.push_back(*j++);
	return res;
}

inline int sign(double d) { return (d>0?1:-1); }

int main()
{
    ifstream file("rcv1_train_reduced.binary", ios::in | ios::binary);
    if (!file) {
        return 1;
    }

    string line;
    int lineCount = 0;
	vector <vp> x(N + 5);
	vector <int> y(N + 5);
    while (lineCount++ < N && getline(file, line))
	{
        istringstream iss(line);
        
        int index;
        double value;
		iss >> y[lineCount];
        while (iss >> index)
		{
            char colon;
            iss >> colon;
            iss >> value;
            x[lineCount].push_back({index, value});
        }
    }
    file.close();
    struct timeval tv;
	gettimeofday(&tv, NULL);
	srand(tv.tv_sec * 1000 + tv.tv_usec / 1000);
	vector <double> norm_record;
	for(int run = 0; run < 2000; run ++)
	{
		int correctTime = 0, totalFix = 0, totalTime = 0;
		vp w(N + 5);
		
		norm_record.clear();
		while(correctTime < 5 * N)
		{
			correctTime ++;
			totalTime ++;
			int r = rand() % N+1;
			if(sign(innerProduct(w, x[r])) != y[r])
			{
				correctTime = 0;
				totalFix ++;
				if(y[r] == -1)
				{
					auto tmp = x[r];
					for(auto &i:tmp) i.second *= -1;
					w = add(w, tmp);
				}
				else w = add(w, x[r]);
				
				double norm = 0;
				for(auto i:w)
	    			norm += i.second * i.second;
				norm_record.push_back(sqrt(norm));
			}
		}
		
		cout << totalFix << ' ';
		for(auto i:norm_record)	
			cout << i << ' ';
		cout << endl;
	}
	
    
    return 0;
}

