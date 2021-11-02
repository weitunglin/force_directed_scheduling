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
  this->m_alap = -1;
  this->m_asap = -1;
}

int Node::mobility() {
  return this->m_alap - this->m_asap;
}

Blif::Blif(std::string filename, int latencyConstraint) : m_latencyConstraint(latencyConstraint) {
  this->parseFile(filename);
  for (uint i = AND; i <= NOT; ++i) {
    this->m_resource[i] = 1;
  }
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

  this->m_total = this->m_graph.size();
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

void Blif::ALAP() {
  for (auto i = std::begin(this->m_graph); i != std::end(this->m_graph); ++i) {
    i->second->m_alap = -1;
  }

  int count = 0;
  std::vector<std::string> readyList;
  for (const auto& i: this->m_outputs) {
    ++count;
    this->m_graph[i]->m_alap = this->m_latencyConstraint;
    std::copy(std::begin(this->m_graph[i]->m_prev), std::end(this->m_graph[i]->m_prev), std::back_inserter(readyList));
  }

  while (count < this->m_total) {
    std::set<std::string> nextList;
    for (auto i = std::begin(readyList); i != std::end(readyList); ++i) {
      bool ready = this->m_graph[*i]->m_alap == -1 && std::all_of(std::begin(this->m_graph[*i]->m_next), std::end(this->m_graph[*i]->m_next), [&] (const std::string& u) { return this->m_graph[u]->m_alap != -1; });
      if (ready) {
        ++count;
        int min = std::numeric_limits<int>::max();
        if (this->m_graph[*i]->m_step == 0) {
          min = 0;
        }

        for (const auto& j: this->m_graph[*i]->m_next) {
          min = std::min(min, this->m_graph[j]->m_alap - 1);
        }
        
        if (min <= -1) {
          std::cerr << "no feasible solution" << std::endl;
          return;
        }

        this->m_graph[*i]->m_alap = min;
        for (const auto& j: this->m_graph[*i]->m_prev) {
          nextList.insert(j);
        }
      } else {
        nextList.insert(*i);
      }
    }

    readyList.clear();
    readyList.assign(std::begin(nextList), std::end(nextList));
  }

  std::cout << "ALAP result" << std::endl;
  for (const auto& i: this->m_graph) {
    std::cout << i.first << ": " << i.second->m_alap << std::endl;
  }
}

void Blif::ASAP() {
  for (auto i = std::begin(this->m_graph); i != std::end(this->m_graph); ++i) {
    i->second->m_asap = -1;
  }

  int count = 0;
  std::vector<std::string> readyList;
  for (const auto& i: this->m_inputs) {
    ++count;
    this->m_graph[i]->m_asap = 0;
    std::copy(std::begin(this->m_graph[i]->m_next), std::end(this->m_graph[i]->m_next), std::back_inserter(readyList));
  }

  while (count < this->m_total) {
    std::set<std::string> nextList;
    for (auto i = std::begin(readyList); i != std::end(readyList); ++i) {
      bool ready = this->m_graph[*i]->m_asap == -1 && std::all_of(std::begin(this->m_graph[*i]->m_prev), std::end(this->m_graph[*i]->m_prev), [&] (const std::string& u) { return this->m_graph[u]->m_asap != -1; });
      if (ready) {
        ++count;
        int max = INT_MIN;
        for (const auto& j: this->m_graph[*i]->m_prev) {
          max = std::max(max, this->m_graph[j]->m_asap + 1);
        }
        
        this->m_graph[*i]->m_asap = max;
        for (const auto& j: this->m_graph[*i]->m_next) {
          nextList.insert(j);
        }
      } else {
        nextList.insert(*i);
      }
    }

    readyList.clear();
    readyList.assign(std::begin(nextList), std::end(nextList));
  }

  std::cout << "ASAP result" << std::endl;
  for (const auto& i: this->m_graph) {
    std::cout << i.first << ": " << i.second->m_asap << std::endl;
  }
}

void Blif::updateDist() {
  // zero dist
  for (uint op = AND; op <= NOT; ++op) {
    for (int i = 0; i <= this->m_latencyConstraint; ++i) {
      this->m_dist[op][i] = 0.0;
    }
  } 

  // compute opeartion distribution
  for (const auto& i: this->m_graph) {
    int slack = i.second->m_alap - i.second->m_asap;
    if (slack == 0) {
      continue;
    }

    double p = 1 / ((slack) + 1.0);
    for (int step = i.second->m_asap; step <= i.second->m_alap; ++step) {
      this->m_dist[i.second->m_op][step] += p;
    }
  }
}

int Blif::FD_LCS() {
  for (const auto& i: this->m_inputs) {
    this->m_graph[i]->m_step = 0;
  }

  this->ALAP();
  this->ASAP();

  std::set<std::string> readyList;
  std::map<uint, std::map<int, int>> resourceCount;
  for (uint op = AND; op <= NOT; ++op) {
    for (int i = 0; i < this->m_latencyConstraint; ++i) {
      resourceCount[op][i] = 0;
    }
  }

  int count = 0;
  count += this->m_inputs.size();
  for (const auto& i: this->m_inputs) {
    this->m_graph[i]->m_step = 0;
    for (const auto& j: this->m_graph[i]->m_next) {
      readyList.insert(j);
    }
  }

  while (count < this->m_total) {
    this->updateDist();
    Node* minForceNode = nullptr;
    double minForce = std::numeric_limits<double>::max();
    int scheduleStep = -1;
    for (auto i = std::begin(readyList); i != std::end(readyList); ++i) {
      Node* n = this->m_graph[*i];

      // check ready
      bool ready = true;
      for (const auto& j: n->m_prev) {
        if (this->m_graph[j]->m_step == -1) {
          ready = false;
        }
      }
 
      if (!ready) {
        continue;
      }

      // check immediate
      if (n->m_alap == n->m_asap) {
        if (resourceCount[n->m_op][n->m_alap] >= this->m_resource[n->m_op]) {
          ++this->m_resource[n->m_op];
        }

        minForceNode = n;
        minForce = std::numeric_limits<double>::min();
        scheduleStep = n->m_alap;
        std::cout << "set imme " << std::endl;
        break;
      }

      int slack = n->m_alap - n->m_asap;
      double p = 1 / ((slack) + 1.0);
      for (int step = n->m_asap; step <= n->m_alap; ++step) {
        double selfForce = 0.0, succForce = 0.0;
        // self force
        for (int t = n->m_asap; t <= n->m_alap; ++t) {
          selfForce += (static_cast<int>(step == t) - p) * this->m_dist[n->m_op][t];
        }

        // successors force
        for (const auto& next: n->m_next) {
          Node* succ = this->m_graph[next];
          double newP = 1 / ((succ->m_alap - step) + 1.0), oldP = 1 / ((succ->m_alap - succ->m_asap) + 1.0);
          double newDist = 0.0, oldDist = 0.0;
          for (int nextStep = succ->m_asap; nextStep <= succ->m_alap; ++nextStep) {
            if (nextStep >= step) {
              newDist += this->m_dist[succ->m_op][nextStep]; 
            }
            
            oldDist += this->m_dist[succ->m_op][nextStep];
          }

          succForce += newP * (newDist) - p * (oldDist);
        }

        double totalForce = selfForce + succForce;
        std::cout << n->m_name << " total force " << totalForce << " self force " << selfForce << " succ force " << succForce << std::endl;
        if (totalForce < minForce && (resourceCount[n->m_op][step] < this->m_resource[n->m_op])) {
          std::cout << "new min " << n->m_name << std::endl;
          minForce = totalForce;
          minForceNode = n;
          scheduleStep = step;
        }
      }
    }

    minForceNode->m_step = scheduleStep;
    ++count;
    std::cout << std::endl << "set " << minForceNode->m_name << " at step " << scheduleStep << ", op " << minForceNode->m_op << ", force " << minForce << std::endl << std::endl;
    ++resourceCount[minForceNode->m_op][minForceNode->m_step];
    this->m_result[minForceNode->m_op][minForceNode->m_step].push_back(minForceNode->m_name);
 
    // remove scheduled node from ready list
    // add successors into ready list
    readyList.erase(std::find(std::begin(readyList), std::end(readyList), minForceNode->m_name));
    for (const auto& i: minForceNode->m_next) {
      readyList.insert(i);
      this->m_graph[i]->m_asap = scheduleStep + 1;
    }
  }

  for (uint op = AND; op <= NOT; ++op) {
    std::cout << "op: " << op << std::endl;
    for (int i = 1; i < this->m_latencyConstraint; ++i) {
      std::cout << "step " << i << ": " << resourceCount[op][i] << std::endl;
    }
  }

  return 0;
}

std::ostream& operator<<(std::ostream& os, Blif& rhs) {
  os << "Latency-constrained Scheduling" << std::endl;
  for (int step = 1; step <= rhs.m_latencyConstraint; ++step) {
    os << step << ": ";
    for (uint op = AND; op <= NOT; ++op) {
      os << "{";
      for (const auto& i: rhs.m_result[op][step]) {
        os << i << " ";
      }
      os << "} ";
    }

    os << std::endl;
  }
  return os;
}
