
#include "ClusteringReader.h"

ClusteringReader::ClusteringReader(std::string clusteringpath, CSR<int, Rule>* rulecsr, Index* index, TraintripleReader* graph) {
	this->graph = graph;
	this->index = index;
	read(rulecsr, clusteringpath);
}

CSR<int, Rule>* ClusteringReader::getCSR() {
	return csr;
}

std::unordered_map<int, std::pair<bool, std::vector<std::vector<int>>>>& ClusteringReader::getRelToClusters() {
	return relToClusters;
}

void ClusteringReader::read(CSR<int, Rule>* rulecsr, std::string clusteringpath) {
	std::unordered_map<std::string, int> rulestringToID;

	int rellen = index->getRelSize();
	Rule* rules_adj_list = rulecsr->getAdjList();
	int* adj_begin = rulecsr->getAdjBegin();
	for (int i = 0; i < rellen; i++) {
		int ind_ptr = adj_begin[3 + i];
		int len = adj_begin[3 + i + 1] - ind_ptr;
		for (int j = 0; j < len; j++) {
			Rule currRule = rules_adj_list[ind_ptr + j];
			rulestringToID[currRule.getRulestring()] = j;
		}
	}

	std::string line;
	std::ifstream myfile(clusteringpath);
	if (myfile.is_open()) {
		int relation;
		while (!util::safeGetline(myfile, line).eof())
		{
			while (line.compare("") == 0) {
				util::safeGetline(myfile, line);
			}

			std::vector<std::string> rel_conf = util::split(line, '\t');
			int relation = *index->getIdOfRelationstring(rel_conf[1]);
			std::vector<std::string> thresholds_str = util::split(rel_conf[2], ' ');
			bool is_max_approach = true;
			for (int q = 0; q < 6; q++) {
				if (std::stod(thresholds_str[q]) > 0.0) {
					is_max_approach = false;
					break;
				}
			}

			std::vector<std::vector<int>> id_clusters;
			while (true) {
				util::safeGetline(myfile, line);
				if (line.compare("") == 0) {
					break;
				}
				std::vector<std::string> rules = util::split(line, '\t');
				std::vector<int> id_cluster;
				for (auto rule : rules) {
					if (rule.compare("") == 0) {
						continue;
					}

					auto id = rulestringToID.find(rule);
					if (id != rulestringToID.end()) {
						id_cluster.push_back(id->second);
					}
					else {
						std::cout << "Rule in cluster not found in ruleset";
						exit(-1);
					}
				}
				id_clusters.push_back(id_cluster);
			}
			relToClusters[relation] = std::make_pair(is_max_approach, id_clusters);
		}
		myfile.close();
	}
	else {
		std::cout << "Unable to open rule file " << clusteringpath << std::endl;
		exit(-1);
	}

}
