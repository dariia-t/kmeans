// Implementation of the KMeans Algorithm
// reference: https://github.com/marcoscastro/kmeans

#include <iostream>
#include <vector>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <random>
#include <tbb/parallel_for.h>
#include <tbb/enumerable_thread_specific.h>

using namespace std;

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

	// NOTES: 
	// tried to compute Euclidean distance incrementally and avoid recalculating the sum each time
	// this did not give me a speed up 

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


	// NOTES:
	// tried to parallelize phase 1
	// this was expected not to give a speed up
	// which is exactly what happened
	// needed a thread safe random number generation:

	// Generates a random int between min and max (inclusive)
	// int generateRandomInt(int min, int max) {
	// 	//thread_local static random_device rd;       // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
	// 	thread_local static mt19937 gen(42);          // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
	// 	uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)
	// 	return distrib(gen);                          // Generate random number from the uniform int dist (inclusive)
	// }

	void run(vector<Point> & points)
	{
        auto begin = chrono::high_resolution_clock::now();

		if(K > total_points)
			return;

		
		// NOTES
		// using tbb significantly slowed down phase 1
		// kept the improved serial version 

		// tbb::concurrent_unordered_set<int> prohibited_indexes;
		// tbb::parallel_for(0, K, [&](int i)
		// {
		// 	while(true)
		// 	{
		// 		int index_point = generateRandomInt(0, total_points - 1);

		// 		if(prohibited_indexes.insert(index_point).second)
		// 		{
		// 			points[index_point].setCluster(i);
		// 			clusters.emplace_back(i, points[index_point]);
		// 			break;
		// 		}
		// 	}
		// });

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
		// NOTES: the variable done is shared across threads and can cause race conditons 
		// using atomic for thread safety 
		std::atomic<bool> done(true); 

		while(true)
		{
			done = true;

			// associates each point to the nearest center
			// CITATION: https://doi.org/10.1016/B978-0-12-415993-8.00011-6
			tbb::parallel_for(0, total_points, [&](int i) 
			{
				// get the current cluster of the point 
				int id_old_cluster = points[i].getCluster();
				// find the nearest center 
				int id_nearest_center = getIDNearestCenter(points[i]);

				if(id_old_cluster != id_nearest_center)
				{
					// reassign the point to that cluster 
					points[i].setCluster(id_nearest_center);
					// if any point changes the cluster we need another iteration 
					done = false;
	            }
			});

			// recalculating the center of each cluster
			// NOTES: struct for intermidiate results
			// to avoid conflicts if multiple threads are trying to update the centers 
			struct LocalVars {
				std::vector<std::vector<double>> centers; 
				std::vector<int> counts; 
				LocalVars(int K, int total_values): centers(K, std::vector<double>(total_values, 0.0)), counts(K, 0) {}
			};

			// https://stackoverflow.com/questions/30275431/tbb-thread-local-set-using-combinable-or-enumerable-thread-specific
			// NOTES: thread safe local storage
			// was considering tbb::combinable as well
			// StackOverflow had an example of enumerable_thread_specific
			tbb::enumerable_thread_specific<LocalVars> local_data(K, total_values);

			tbb::parallel_for(0, total_points, [&](int i) {
				int cluster_id = points[i].getCluster();
				// get thread-local instance of LocalVars
				// no race conditions since each thread has its own local data 
				auto &local = local_data.local();
				// update point count for this cluster
				local.counts[cluster_id]++;
				// accumulate values for this cluster
				for (int j = 0; j < total_values; j++) {
					local.centers[cluster_id][j] += points[i].getValue(j);
				}

				// NOTES: tried loop unrolling 
				// did not observe a speed up

				// for (int j = 0; j < total_values; j += 2) {
				// 	local.centers[cluster_id][j] += points[i].getValue(j);
				// 	if (j + 1 < total_values) {
				// 		local.centers[cluster_id][j + 1] += points[i].getValue(j + 1);
				// 	}
				// }
			});

			// aggregate results from all threads
			std::vector<std::vector<double>> new_centers(K, std::vector<double>(total_values, 0.0));
			std::vector<int> points_per_cluster(K, 0);

			// applies the lambda function to each threads local storage
			local_data.combine_each([&](const LocalVars &local) {
				for (int i = 0; i < K; i++) {
					// sum up total points in each cluster
					points_per_cluster[i] += local.counts[i];  
					// sum up total values for each cluster
					for (int j = 0; j < total_values; j++) {
						new_centers[i][j] += local.centers[i][j];  
					}
				}
			});

			// compute new cluster centers 
			// CITATION: https://doi.org/10.1016/B978-0-12-415993-8.00011-6
			tbb::parallel_for(0, K, [&](int i) {
				if (points_per_cluster[i] > 0) {
					for (int j = 0; j < total_values; j++) {
						clusters[i].setCentralValue(j, new_centers[i][j] / points_per_cluster[i]);
					}
				}
			});

			if(done == true || iter >= max_iterations)
			{
				cout << "Break in iteration " << iter << "\n\n";
				break;
			}

			iter++;
		}
        auto end = chrono::high_resolution_clock::now();


        for (int i = 0; i < K; i++) {

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
