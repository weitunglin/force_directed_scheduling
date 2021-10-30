#ifndef BLIF_H
#define BLIF_H
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

/*
FDS algorithm
repeat {
  run ALAP, ASAP
  compute the opeartion and type prob.
  compute the self-force, ps-force and total force
  scheulde the op. with least force
}
until (all opeartions are schedules)
*/

const uint AND = 0;
const uint OR = 1;
const uint NOT = 2;

class Node {
public:
  Node(std::string name);

protected:
  std::string m_name; // node name
  std::vector<std::string> m_pred; // predecessors
  std::vector<std::string> m_succ; // successors
  int m_step; // scheduled step; -1 => hasn't been scheduled
  int m_alap; // result of ALAP
  int m_asap; // result of ASAP
  int mobility(); // ALAP - ASAP
  uint m_op; // operation type

  double m_selfForce; // self force
  double m_psForce; // ps force
  double totalForce(); // self force + ps force
};

class Blif : public Node {
public:
  Blif(std::string filename, int latency);

private:
  void parseFile();

  std::map<std::string, Node*> m_graph; // contains all nodes (stored in std::map)
  std::map<uint, std::vector<double>> m_dist; // operation type distribution
  int m_latency; // latency contraint
};

#endif
