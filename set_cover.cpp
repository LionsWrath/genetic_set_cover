#include <iostream>
#include <queue>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <random>
#include <utility>
#include <set>
#include <chrono>
#include <functional>

typedef struct{
    std::vector<int> chromossome;
    double fitness;
} individual;

typedef struct {
    std::vector<int> setValues;
    double weight;
    double fitness;
} set;

class comparison {
    public:
        bool operator() (const individual& lhs, const individual& rhs) const {
            return {lhs.fitness > rhs.fitness};
        }
};

//test
class optComparison {
    public:
        bool operator() (const std::pair<int,set>& lhs, const std::pair<int,set>& rhs) const {
            if (lhs.second.setValues.size() == rhs.second.setValues.size()) 
                return (lhs.second.weight < rhs.second.weight);
            else 
                return (lhs.second.setValues.size() > rhs.second.setValues.size());
        }
};

typedef std::priority_queue<individual,std::vector<individual>,comparison> population_heap;
typedef std::pair<individual,individual> individual_pair;
typedef std::vector< std::vector<int> > set_index;
typedef std::vector<set> data;

int N, M;

void toCString(char** cstr, std::string* str) {
    *cstr = new char [str->length()+1];
    strcpy (*cstr, str->c_str());
}

void insertIndex(set_index* indexes, set * new_set, int index) {
    for (int i : new_set->setValues) 
        indexes->operator[](i).push_back(index);
}

set_index readFile(std::string filename, data* datasets) {
    std::ifstream infile(filename);
    std::string line;
    char * cstr, * p;

    //N
    std::getline(infile,line);
    toCString(&cstr, &line);

    std::strtok(cstr," ");
    N = atoi(std::strtok(NULL," "));

    //M
    std::getline(infile,line);
    toCString(&cstr, &line);

    std::strtok(cstr," ");
    M = atoi(std::strtok(NULL," "));

    std::getline(infile,line);

    set_index indexes(N, std::vector<int>());

    while(std::getline(infile,line)) {
        set new_set;

        toCString(&cstr, &line);
       
        std::strtok(cstr, " ");
        p = std::strtok(NULL, " ");
        new_set.weight = std::stod(p);
        p = std::strtok(NULL, " ");
        do {
            new_set.setValues.push_back(atoi(p) - 1);
            p = std::strtok(NULL, " ");  
        } while(p != NULL);

        new_set.fitness = new_set.weight/(double)new_set.setValues.size();

        datasets->push_back(new_set);
        insertIndex(&indexes, &new_set, datasets->size()-1);
    }

    return indexes;
}

void printDataset(data dataset) {
    for (auto s : dataset) {
        for (auto i : s.setValues)
            std::cout << i << std::endl;
        std::cout << s.weight << std::endl;
    }
}

void printIndividual(individual ind) {
    for (int i : ind.chromossome)
        std::cout << i << " ";
    std::cout << std::endl << ind.fitness << std::endl;  
}

void printIndexes(set_index indexes) {
    for (auto vec : indexes) {
        for (auto i : vec)
            std::cout << i << " ";

        std::cout << std::endl;
    }
    std::cout << "end" << std::endl;
}

void testRandomGenerator(std::mt19937 * random_generator, int generations) { 
    for (int i=0; i< generations; i++)
        std::cout << " " << (*random_generator)() << " ";
}

double individualWeight(individual * ind, data * datasets) {
    std::set<int> unique_set;
    double weight = 0;
    
    for (int i : ind->chromossome)
        unique_set.insert(i);

    for (int i : unique_set) 
        weight += datasets->at(i).weight;

    return weight;
}

//Test 
void optimizeIndividual(individual * ind, data * datasets) {
     std::priority_queue<std::pair<int,set>, std::vector< std::pair<int,set> >, optComparison> red_set;
     std::set<int> unique_set;
     for (int i : ind->chromossome)
        unique_set.insert(i);

     for (int i : unique_set)
        red_set.push(std::make_pair(i, datasets->at(i)));

     while (!red_set.empty()) {
         std::pair<int,set> actual = red_set.top();
        red_set.pop();

        for (auto i : actual.second.setValues)
            ind->chromossome.at(i) = actual.first;
     }
}

void calculateFitness(data * datasets, individual * ind) {
    std::set<int> unique_set;
    
    for (int i : ind->chromossome)
        unique_set.insert(i);

    std::cout << "FIT: " << unique_set.size() << " " << ind->chromossome.size() << std::endl;

    ind->fitness = 0;
    for (int i : unique_set) {
        ind->fitness += datasets->at(i).fitness;
    }
}

void createIndividual(individual * new_ind, std::mt19937 * random_generator, set_index * indexes, 
        data * datasets) {
    double weight = 0;
    double elements = 0;
    
    for (int i=0; i<N; i++) {
        int set_value = indexes->at(i)[(*random_generator)() % indexes->at(i).size()];
        new_ind->chromossome.push_back(set_value);
    }

    //Test
    optimizeIndividual(new_ind, datasets);
    calculateFitness(datasets, new_ind);
}

void initializePopulation(int size, population_heap * population, std::mt19937 * random_generator, 
        set_index * indexes, data * datasets) {
    for (int i=0; i<size; i++) {
        individual new_ind;
        createIndividual(&new_ind, random_generator, indexes, datasets); 
        population->push(new_ind);
    }
}

std::pair<individual,individual> chooseParents(population_heap * population) {
    individual par1 = population->top();
    population->pop();
    individual par2 = population->top();
    population->pop();

    return std::make_pair(par1,par2);
}

//One-Point Crossover
void crossover1(std::pair<individual, individual> * parents, data * datasets, 
        std::mt19937 * random_generator, std::pair<individual, individual> * children) {
    int k = (*random_generator)() % N;

    std::cout << "Random: "<< k << std::endl;
    for (k; k<N; k++) {
        children->first.chromossome.at(k) = parents->second.chromossome[k];
        children->second.chromossome.at(k) = parents->first.chromossome[k];
    }

    //Testing
    optimizeIndividual(&(children->first), datasets);
    optimizeIndividual(&(children->second), datasets);

    calculateFitness(datasets, &(children->first));
    calculateFitness(datasets, &(children->second));
}

//Uniform Crossover
void crossover(std::pair<individual, individual> * parents, data * datasets, 
        std::mt19937 * random_generator, std::pair<individual, individual> * children) {

    for (int k = 0; k<N; k++) {
        if ((*random_generator)() % 100 > 49) {
            children->first.chromossome.at(k) = parents->second.chromossome[k];
            children->second.chromossome.at(k) = parents->first.chromossome[k]; 
        }
    }

    //Testing
    optimizeIndividual(&(children->first), datasets);
    optimizeIndividual(&(children->second), datasets);

    calculateFitness(datasets, &(children->first));
    calculateFitness(datasets, &(children->second));
}

//Can be better -- not changing
void mutation(std::mt19937 * random_generator, individual * children, set_index * indexes, data * datasets) {
    if ((*random_generator)() % 100 < (5)) {
        int set_index = (*random_generator)() % N;
        int set_size = indexes->at(set_index).size();

        printIndividual(*children);

        int set_actual = children->chromossome.at(set_index);
        int set_change = (*random_generator)() % set_size;
        children->chromossome.at(set_index) = indexes->at(set_index)[ set_change ];

        std::cout << "MUTATE: " << set_index << " FROM: " << set_actual << " TO: " 
            << indexes->at(set_index)[set_change] << std::endl;

        optimizeIndividual(children, datasets);
        calculateFitness(datasets, children);
    }
}

//steady_state management
void managePopulation(std::pair<individual, individual> * parents, std::pair<individual, individual> * children, 
        population_heap * population) {
    population_heap steady_state;

    steady_state.push(parents->first);
    steady_state.push(parents->second);
    steady_state.push(children->first);
    steady_state.push(children->second);

    //steady_state.pop();
    //steady_state.pop();
    population->push(steady_state.top());
    steady_state.pop();
    population->push(steady_state.top());

}

int main() {
    std::mt19937::result_type seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937::result_type testseed = 1598659863;
    std::mt19937 random_generator(testseed);

    testRandomGenerator(&random_generator, 50);

    data datasets;
    set_index indexes = readFile("test_01.dat", &datasets);

    population_heap population;
    initializePopulation(500, &population, &random_generator, &indexes, &datasets);

    int i = 0;

    while (i != 1000) {
        std::cout << "cycle: " << i << std::endl;
        std::pair<individual, individual> parents = chooseParents(&population);

        std::pair<individual, individual> children(parents);

        crossover(&parents, &datasets, &random_generator, &children);

        mutation(&random_generator, &(children.first), &indexes, &datasets);
        mutation(&random_generator, &(children.second), &indexes, &datasets);

        std::cout << "Parents>>>>>>>>>>>" << std::endl;
        printIndividual(parents.first);
        printIndividual(parents.second);

        std::cout << "Children>>>>>>>>>>" << std::endl;
        printIndividual(children.first);
        printIndividual(children.second);

        std::cout << "------------------" << std::endl;

        managePopulation(&parents, &children, &population);

        i++;
    }

    individual ind = population.top();
    std::cout << individualWeight(&(ind), &datasets) << std::endl;
}

