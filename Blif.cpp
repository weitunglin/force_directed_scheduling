#include "Blif.h"

std::string token(std::string s, std::string delim) {
  if (s.find(delim) == std::string::npos) {
    return s;
  }

  return s.substr(s.find(delim) + delim.length(), s.length());
}

std::vector<std::string> split(std::string s) {
  std::stringstream ss(s);
  std::string v;
  std::vector<std::string> res;

  while (ss >> v) {
    res.push_back(v);
  }

  return res;
}

Node::Node(std::string name) : m_name(name) {
  this->m_step = -1;
}

int Node::mobility() {
  return this->m_alap - this->m_asap;
}

Blif::Blif(std::string filename, int latencyConstraint) : m_latency(latencyConstraint) {
  this->parseFile(filename);
}

void Blif::parseFile(std::string filename) {
  std::ifstream f(filename);
  if (!f.is_open()) {
    std::cerr << "Can not open file correctly\n";
    return;
  }

  std::string line;
  do {
    getline(f, line);
  } while (line.find(".model") == std::string::npos);

  this->m_inputs = getList(f, ".inputs ");
  this->m_outputs = getList(f, ".outputs ");

  while (getline(f, line)) {
    if (line.find(".end") != std::string::npos) {
      break;
    }

    if (line.find(".names") != std::string::npos) {
      std::vector<std::string> names = split(token(line, ".names "));
      std::vector<std::string> functions;
 
      while (f.peek() != '.') {
        std::string l;
        getline(f, l);
        functions.push_back(l);
      }

      // identify operation type
      // +, *, !
      uint op;
      if (functions.size() == 1) {
        if (functions[0].front() == '0' && functions[0].back() == '1') {
          op = NOT;
        } else {
          op = AND;
        }
      } else {
        op = OR;
      }

      Node* result = nullptr;
      result = new Node(names.back());
      result->m_op = op;
      this->m_graph[names.back()] = result;

      for (size_t i = 0; i < names.size() - 1; ++i) {
        Node* prev = nullptr;
        if (this->m_graph.count(names[i])) {
          prev = this->m_graph[names[i]];
        } else {
          prev = new Node(names[i]);
          this->m_graph[names[i]] = prev;
        }

        prev->m_next.push_back(names.back());
        result->m_prev.push_back(names[i]);
      }
    }
  }
}

std::vector<std::string> Blif::getList(std::ifstream& f, std::string name) {
  std::vector<std::string> result;
  std::string line;
  bool cont = false;

  do {
    getline(f, line);
    std::vector<std::string> tokened = split(token(line, name));
    if (tokened.back() == "\\") {
      tokened.pop_back();
      cont = true;
    } else {
      cont = false;
    }

    std::copy(std::begin(tokened), std::end(tokened), std::back_inserter(result));
  } while (cont);

  return result;
}

int Blif::FD_LCS() {
  return 0;
}

std::ostream& operator<<(std::ostream& os, const Blif& rhs) {
  return os;
}