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
typedef std::multiset<individual, comparison> population_set;

typedef std::pair<individual, individual> individual_pair;
typedef std::vector< std::vector<int> > set_index;
typedef std::vector<set> data;

int N, M;

//std::random_device rd;
std::mt19937::result_type seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
std::mt19937::result_type testseed = 5896541236589856;

std::mt19937 random_generator(testseed);
std::uniform_int_distribution<int> distribution(0,100);
std::uniform_real_distribution<double> real_distribution(0,1);

auto real_rand = std::bind(real_distribution, random_generator);
auto dice_rand = std::bind(distribution, random_generator);

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
            std::cout << i << " ";
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
        std::cout << "Individual: " << std::endl;
        for (auto i : vec)
            std::cout << i << " ";

        std::cout << std::endl;
    }
    std::cout << "end" << std::endl;
}

void printPopulation(population_set population) {
    for (auto i : population) {
        for (auto j : i.chromossome)
            std::cout << j << " ";
        std::cout << std::endl << i.fitness << std::endl;
    }
}

void testRandomGenerator(int generations) { 
    for (int i=0; i< generations; i++)
        std::cout << " " << random_generator() << " ";
    std::cout << std::endl;
}

double convergenceRate(population_set * population) {
    double distance = population->begin()->fitness - population->rbegin()->fitness;

    return 100 - (distance/population->begin()->fitness)*100;
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

    ind->fitness = 0;
    for (int i : unique_set) {
        ind->fitness += datasets->at(i).fitness;
    }
}

void createIndividual(individual * new_ind, set_index * indexes, data * datasets) {
   
    for (int i=0; i<N; i++) {
        int set_value = indexes->at(i)[dice_rand() % indexes->at(i).size()];
        new_ind->chromossome.push_back(set_value);
    }
    
    optimizeIndividual(new_ind, datasets);
    calculateFitness(datasets, new_ind);
}

void initializePopulation(int size, population_set * population, set_index * indexes, data * datasets) {

    for (int i=0; i<size; i++) {
        individual new_ind;
        createIndividual(&new_ind, indexes, datasets); 
        population->insert(new_ind);
    }
}

individual_pair chooseParents(population_set * population) {
    std::vector<std::pair<double, individual>> probability_vector;

    double total_weight = 0.0f;
    individual par1, par2;

    for (auto i : *population) 
        total_weight += i.fitness;

    double acc = 0.0f;
    for (auto s : *population) {
        acc +=  s.fitness/total_weight;
        probability_vector.push_back(std::make_pair(acc, s));
    }

    double s1 = real_rand();
    double s2 = real_rand();

    double last = 0.0f;
    for (int i = 0; i < probability_vector.size(); i++) { 
        if (last < s1 && probability_vector[i].first >= s1) {
            par1 = probability_vector[i].second;
        }
        if (last < s2 && probability_vector[i].first >= s2) {
            par2 = probability_vector[i].second;
        }
        last = probability_vector[i].first;
    } 

    return std::make_pair(par1,par2);
}

//One-Point Crossover
void crossover1(individual_pair * parents, data * datasets, 
        individual_pair * children) {
    int k = random_generator() % N;

    std::cout << "Random: "<< k << std::endl;
    for (k; k<N; k++) {
        children->first.chromossome.at(k) = parents->second.chromossome[k];
        children->second.chromossome.at(k) = parents->first.chromossome[k];
    }

    optimizeIndividual(&(children->first), datasets);
    optimizeIndividual(&(children->second), datasets);

    calculateFitness(datasets, &(children->first));
    calculateFitness(datasets, &(children->second));
}

//Uniform Crossover
void crossover(individual_pair * parents, data * datasets, 
        individual_pair * children) {

    for (int k = 0; k<N; k++) {
        if (dice_rand() > 49) {
            children->first.chromossome.at(k) = parents->second.chromossome[k];
            children->second.chromossome.at(k) = parents->first.chromossome[k]; 
        }
    }

    optimizeIndividual(&(children->first), datasets);
    optimizeIndividual(&(children->second), datasets);

    calculateFitness(datasets, &(children->first));
    calculateFitness(datasets, &(children->second));
}

//Can be better -- not changing
void mutation(individual * children, set_index * indexes, data * datasets, int tx) {
    if (dice_rand() < tx) {
        int set_index = random_generator() % N;
        int set_size = indexes->at(set_index).size();

        printIndividual(*children);

        int set_actual = children->chromossome.at(set_index);
        int set_change = random_generator() % set_size;
        children->chromossome.at(set_index) = indexes->at(set_index)[ set_change ];

        std::cout << "MUTATE: " << set_index << " FROM: " << set_actual << " TO: " 
            << indexes->at(set_index)[set_change] << std::endl;

        optimizeIndividual(children, datasets);
        calculateFitness(datasets, children);
    }
}

//steady_state management
void managePopulation(individual_pair * parents, individual_pair * children, population_set * population, 
        set_index * indexes, data * datasets) {
    population_heap steady_state;

    steady_state.push(parents->first);
    steady_state.push(parents->second);
    steady_state.push(children->first);
    steady_state.push(children->second);

    //steady_state.push(*population->rbegin());
    //steady_state.push(*population->rbegin());

    //population->clear();
    initializePopulation(250, population, indexes, datasets);

    population->insert(steady_state.top());
    steady_state.pop();
    population->insert(steady_state.top());
    steady_state.pop();
    population->insert(steady_state.top());
    steady_state.pop();
    population->insert(steady_state.top());
}

void printStatus(individual_pair * parents, individual_pair * children) {
    std::cout << "Parents>>>>>>>>>>>" << std::endl;
    printIndividual(parents->first);
    printIndividual(parents->second);

    std::cout << "Children>>>>>>>>>>" << std::endl;
    printIndividual(children->first);
    printIndividual(children->second);

    std::cout << "------------------" << std::endl;
}

int main() {
    data datasets;
    set_index indexes = readFile("test_03.dat", &datasets);

    std::cout << "Seed: " << seed << std::endl;
    std::cout << "Seed de Teste: " << testseed << std::endl;

    population_set population;
    initializePopulation(500, &population, &indexes, &datasets);
    
    int i = 0;
    double r, initial = convergenceRate(&population);
    double limit = 80.0f;

    while (i != 1000 && r < limit) {
        std::cout << "cycle: " << i << " Convergence: " << r <<std::endl;
        
        individual_pair parents = chooseParents(&population);

        individual_pair children(parents);
        crossover(&parents, &datasets, &children);

        mutation(&(children.first), &indexes, &datasets, 5/(limit/100));
        mutation(&(children.second), &indexes, &datasets, 5/(limit/100));

        managePopulation(&parents, &children, &population, &indexes, &datasets);

        r = convergenceRate(&population);

        printStatus(&parents, &children);
        i++;
    }

    individual ind = *(population.rbegin());
    std::cout << individualWeight(&ind, &datasets) << " " << initial << " " 
        << convergenceRate(&population)<< std::endl;
}

