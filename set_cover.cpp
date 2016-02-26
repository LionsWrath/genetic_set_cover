#include <iostream>
#include <queue>
#include <fstream>
#include <vector>
#include <cstring>
#include <string>

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

void readFile(std::string filename, data* datasets) {
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

    set_index indexes(N+1, std::vector<int>(N+1));

    while(std::getline(infile,line)) {
        set new_set;

        toCString(&cstr, &line);
       
        std::strtok(cstr, " ");
        p = std::strtok(NULL, " ");
        new_set.weight = std::stod(p);
        p = std::strtok(NULL, " ");
        do {
            std::cout << p << " ";
            new_set.setValues.push_back(atoi(p));
            p = std::strtok(NULL, " ");  
        } while(p != NULL);
        std::cout << std::endl;
        datasets->push_back(new_set);
        insertIndex(&indexes, &new_set, datasets->size()-1);
    }
}

individual createIndividual() {
    individual new_ind;


}

int main() {
    data datasets;
    readFile("test_01.dat", &datasets);

    population_heap population;

    std::cout << datasets.size() << std::endl;
}

