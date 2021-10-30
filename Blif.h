#ifndef BLIF_H
#define BLIF_H
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

const uint AND = 0;
const uint OR = 1;
const uint NOT = 2;

// Utils Functions
std::string token(std::string s, std::string delim);
std::vector<std::string> split(std::string s);

class Node {
public:
  Node(std::string name);
  friend class Blif;

private:
  std::string m_name; // node name
  std::vector<std::string> m_prev; // predecessors
  std::vector<std::string> m_next; // successors
  int m_step; // scheduled step; -1 => hasn't been scheduled
  int m_alap; // result of ALAP
  int m_asap; // result of ASAP
  int mobility(); // ALAP - ASAP
  uint m_op; // operation type

  double m_selfForce; // self force
};

class Blif {
public:
  Blif(std::string filename, int latency);

  /*
  run ALAP check if is feasible
  do {
    compute ALAP, ASAP
    get ready nodes
    compute operation type distribution
    compute ready nodes' total force
    schedule operation
  } while (all operations are schedules)
  */
  int FD_LCS();
  friend std::ostream& operator<<(std::ostream& os, const Blif& rhs);

private:
  void parseFile(std::string filename);
  std::vector<std::string> getList(std::ifstream& f, std::string name);

  std::map<std::string, Node*> m_graph; // contains all nodes (stored in std::map)
  std::map<uint, std::vector<double>> m_dist; // operation type distribution
  std::vector<std::string> m_inputs;
  std::vector<std::string> m_outputs;
  int m_latency; // latency contraint
};

#endif
