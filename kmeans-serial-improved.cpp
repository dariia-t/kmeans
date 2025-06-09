// Implementation of the KMeans Algorithm
// reference: https://github.com/marcoscastro/kmeans

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <chrono>

using namespace std;

// NOTES: point has id of the cluster it belongs to 
// cluster used to have the vector of points 
// this is redundant so i decided to keep the id_cluster in point
// and remove the vector of points in cluster
class Point
{
private:
	int id_point, id_cluster;
	vector<double> values;
	string name;

public:
	Point(int id_point, vector<double>& values, string name = "")
	{
		this->id_point = id_point;
		int total_values = values.size();

		for(int i = 0; i < total_values; i++)
			this->values.push_back(values[i]);

		this->name = name;
		id_cluster = -1;
	}

	int getID()
	{
		return id_point;
	}

	void setCluster(int id_cluster)
	{
		this->id_cluster = id_cluster;
	}

	int getCluster()
	{
		return id_cluster;
	}

	double getValue(int index)
	{
		return values[index];
	}

	int getTotalValues()
	{
		return values.size();
	}

	void addValue(double value)
	{
		values.push_back(value);
	}

	string getName()
	{
		return name;
	}
};


// NOTES: I removed the vector that holds points for the cluster
// so i didnt eed remove and add point anymore either 

class Cluster
{
private:
	int id_cluster;
	vector<double> central_values;

public:
	Cluster(int id_cluster, Point point)
	{
		this->id_cluster = id_cluster;

		int total_values = point.getTotalValues();
		central_values.resize(total_values);
	
		for (int i = 0; i < total_values; i++) {
			central_values[i] = point.getValue(i);
		}
	}

	double getCentralValue(int index)
	{
		return central_values[index];
	}

	void setCentralValue(int index, double value)
	{
		central_values[index] = value;
	}

	int getID()
	{
		return id_cluster;
	}
};

class KMeans
{
private:
	int K; // number of clusters
	int total_values, total_points, max_iterations;
	vector<Cluster> clusters;

	// return ID of nearest center (uses euclidean distance)
	int getIDNearestCenter(Point point)
	{
		double sum = 0.0, min_dist;
		int id_cluster_center = 0;

		for(int i = 0; i < total_values; i++)
		{
			sum += pow(clusters[0].getCentralValue(i) -
					   point.getValue(i), 2.0);
		}

		min_dist = sqrt(sum);

		for(int i = 1; i < K; i++)
		{
			double dist;
			sum = 0.0;

			for(int j = 0; j < total_values; j++)
			{
				sum += pow(clusters[i].getCentralValue(j) -
						   point.getValue(j), 2.0);
			}

			dist = sqrt(sum);

			if(dist < min_dist)
			{
				min_dist = dist;
				id_cluster_center = i;
			}
		}

		return id_cluster_center;
	}

public:
	KMeans(int K, int total_points, int total_values, int max_iterations)
	{
		this->K = K;
		this->total_points = total_points;
		this->total_values = total_values;
		this->max_iterations = max_iterations;
	}

	void run(vector<Point> & points)
	{
        auto begin = chrono::high_resolution_clock::now();

		if(K > total_points)
			return;

		vector<int> prohibited_indexes;

		// choose K distinct values for the centers of the clusters
		for(int i = 0; i < K; i++)
		{
			while(true)
			{
				int index_point = rand() % total_points;

				if(find(prohibited_indexes.begin(), prohibited_indexes.end(),
						index_point) == prohibited_indexes.end())
				{
					prohibited_indexes.push_back(index_point);
					points[index_point].setCluster(i);
					clusters.emplace_back(i, points[index_point]);
					break;
				}
			}
		}
		
        auto end_phase1 = chrono::high_resolution_clock::now();

		int iter = 1;

		while(true)
		{
			bool done = true;

			// associates each point to the nearest center
			for(int i = 0; i < total_points; i++)
			{
				int id_old_cluster = points[i].getCluster();
				int id_nearest_center = getIDNearestCenter(points[i]);

				if(id_old_cluster != id_nearest_center)
				{
					// set the cluster for the point 
					points[i].setCluster(id_nearest_center);
					done = false;
				}
			}

			// recalculating the center of each cluster
			// initialize the new centers
            vector<vector<double>> new_centers(K, vector<double>(total_values, 0.0));
            // intitialize new point counters
			vector<int> points_per_cluster(K, 0);

			// sum up values for each cluster by getting the cluster id of each point
            for (int i = 0; i < total_points; i++) {
                int cluster_id = points[i].getCluster();
                points_per_cluster[cluster_id]++;
                for (int j = 0; j < total_values; j++) {
                    new_centers[cluster_id][j] += points[i].getValue(j);
                }
            }
			// compute the new centroid 
            for (int i = 0; i < K; i++) {
                if (points_per_cluster[i] > 0) {
                    for (int j = 0; j < total_values; j++) {
                        clusters[i].setCentralValue(j, new_centers[i][j] / points_per_cluster[i]);
                    }
                }
            }

			if(done == true || iter >= max_iterations)
			{
				cout << "Break in iteration " << iter << "\n\n";
				break;
			}

			iter++;
		}
        auto end = chrono::high_resolution_clock::now();


        for (int i = 0; i < K; i++) {
            // cout << "Cluster " << clusters[i].getID() + 1 << endl;
            // for (int j = 0; j < total_points; j++) {
            //     if (points[j].getCluster() == clusters[i].getID()) {
            //         cout << "Point " << points[j].getID() + 1 << ": ";
            //         for (int p = 0; p < total_values; p++)
            //             cout << points[j].getValue(p) << " ";
            //         if (!points[j].getName().empty())
            //             cout << "- " << points[j].getName();
            //         cout << endl;
            //     }
            // }

            cout << "Cluster values: ";
            for (int j = 0; j < total_values; j++)
                cout << clusters[i].getCentralValue(j) << " ";
            cout << "\n\n";
        }


		cout << "TOTAL EXECUTION TIME = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-begin).count()<<"\n";

		cout << "TIME PHASE 1 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end_phase1-begin).count()<<"\n";

		cout << "TIME PHASE 2 = "<<std::chrono::duration_cast<std::chrono::microseconds>(end-end_phase1).count()<<"\n";

		cout << "TIME PER ITERATION = "<<std::chrono::duration_cast<std::chrono::microseconds>((end-end_phase1)/iter).count()<<"\n";
	
	}
};

int main(int argc, char *argv[])
{
	srand (42);

	int total_points, total_values, K, max_iterations, has_name;

	cin >> total_points >> total_values >> K >> max_iterations >> has_name;

	vector<Point> points;
	string point_name;

	for(int i = 0; i < total_points; i++)
	{
		vector<double> values;

		for(int j = 0; j < total_values; j++)
		{
			double value;
			cin >> value;
			values.push_back(value);
		}

		if(has_name)
		{
			cin >> point_name;
			Point p(i, values, point_name);
			points.push_back(p);
		}
		else
		{
			Point p(i, values);
			points.push_back(p);
		}
	}

	KMeans kmeans(K, total_points, total_values, max_iterations);
	kmeans.run(points);

	return 0;
}
