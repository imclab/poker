#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <omp.h>

#include "NeuralNetwork.h"
#include "Phenotype.h"
#include "PopulationEvolver.h"
#include <cmath>

vector< pair<char, vector<double> > > dataset;

vector<int> choice;
int last_generation = -2;

double evaluate_fitness (NeuralNetwork* network, int generation) {
    if (last_generation != generation) {
        #pragma omp single
        {
            if (rand() % 100 == 0 || last_generation == -2) {
                choice.clear();
                choice.resize(500);
                for (int i = 0; i < choice.size(); i++) {
                    choice[i] = rand() % 16000;
                }
                cout << "--------------" << endl;
            }
        }
        
        #pragma omp barrier
        
        #pragma omp single
        {
            last_generation = generation;
        }
    }
    
    double predictions = 0.0;
    for (int r = 0; r < choice.size(); r++) {
        vector<double> output = network->get_output(dataset[choice[r]].second);
        
        int letter = dataset[choice[r]].first - 65;
        
        int max = 0;
        double max_value = -1;
        for (int j = 0; j < output.size(); j++) {
            if (output[j] > max_value) {
                max = j;
                max_value = output[j];
            }
        }
        
        if (max == letter) {
            predictions++;
        }
    }
      
    return predictions;
}

int main (int argc, char * const argv[]) {
    fstream file;
    
    file.open("Dataset.data");
    
    dataset.resize(20000);
    for (int i = 0; i < 20000; i++) {
        file >> dataset[i].first;
        dataset[i].second.resize(16);
        for (int j = 0; j < 16; j++) {
            file >> dataset[i].second[j];
        }
    }
    
    PopulationEvolver ev(16, 26, evaluate_fitness, 1000, 50, 12, 0.01, true);
    
    for (int i = 0; i < ev.population_.size(); i++) {
        ifstream save;
        
        stringstream filename;
        filename << "networks/network" << i << ".gv";
        save.open(filename.str().c_str());
        
        if (save.is_open()) {
            string current = "";
            while (current != "input_units:") {
                save >> current;
            }
            save >> ev.population_[i]->input_units_;
            
            while (current != "output_units:") {
                save >> current;
            }
            save >> ev.population_[i]->output_units_;
            
            save >> current;
            for (int j = 0; current != "*/"; j++) {
                ev.population_[i]->genes_[j] = atoi(current.c_str());
                save >> current;
            }
        }
    }
    
    for (int g = 0; g < 10000; g++) {
        ev.evolve(100);
        
        ev.get_population_fitness(-1);
    
        cerr << g << " - Best fitness = " << ev.population_[0]->fitness_;
        
        double average_neurons = 0.0;
        double average_fitness = 0.0;
        for (int i = 0; i < ev.population_.size(); i++) {
            average_neurons += ev.population_[i]->network_->hidden_neurons_.size();
            average_fitness += ev.population_[i]->fitness_;
        
            stringstream filename;
            filename << "networks/network" << i << ".gv";
        
            ofstream file;
            file.open(filename.str().c_str());
            file << ev.population_[i]->save();
            file.close();
        }
        
        average_neurons = average_neurons / ev.population_.size();
        average_fitness = average_fitness / ev.population_.size();
        
        int predicted = 0;
        for (int i = 16000; i < dataset.size(); i++) {
            NeuralNetwork* network = ev.population_[0]->network_;
            vector<double> output = network->get_output(dataset[i].second);

            int max = 0;
            double max_value = -1;
            for (int j = 0; j < output.size(); j++) {
                if (output[j] > max_value) {
                    max = j;
                    max_value = output[j];
                }
            }

            if (max == (dataset[i].first - 65)) {
                predicted++;
            }
        }
    
        cout << ", Average neurons = " << average_neurons << ", Average fitness = " << average_fitness << ", Correctly predicted = " << predicted << endl;
    }
    
    return 0;
}
