#include <iostream>
#include <queue>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>
#include <random>
#include <utility>
#include <set>

typedef struct{
    std::vector<int> chromossome;
    double fitness;
} individual;

typedef struct {
    std::vector<int> setValues;
    double weight;
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
    int weight = 0;
    int elements = 0;
    
    for (int i : ind->chromossome)
        unique_set.insert(i);

    for (int i : unique_set) {
        weight += datasets->at(i).weight;
        elements += datasets->at(i).setValues.size();
    }
    ind->fitness = (elements - 50.0f)*weight;
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

void crossover(std::pair<individual, individual> * parents, data * datasets, 
        std::mt19937 * random_generator, std::pair<individual, individual> * children) {
    int k = (*random_generator)() % N;
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

void mutation(std::mt19937 * random_generator, individual * children) {
	if ((*random_generator)() % 100 < (1.0f/(float)N)) {
		//put the mutation here
	}
}

int main() {
    std::mt19937::result_type seed = 125689566;
    std::mt19937 random_generator(seed);

    data datasets;
    set_index indexes = readFile("test_01.dat", &datasets);

    //std::cout << datasets.size() << std::endl;

    //printDataset(datasets);
    //printIndexes(indexes);

    //individual ind; 
    //createIndividual(&ind ,&random_generator, &indexes, &datasets);
    
    //printIndividual(ind);  
   
    population_heap population;
    initializePopulation(500, &population, &random_generator, &indexes, &datasets);

    //std::cout << population.size() << std::endl;

    std::pair<individual, individual> parents = chooseParents(&population);

    printIndividual(parents.first);
    printIndividual(parents.second);

    std::pair<individual, individual> children(parents);

    crossover(&parents, &datasets, &random_generator, &children);

    printIndividual(children.first);
    printIndividual(children.second);

    mutation(&random_generator, &(children.first));
    mutation(&random_generator, &(children.second));
}

