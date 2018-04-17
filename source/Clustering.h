//
// Created by cicerolp on 4/17/18.
//

#pragma once

class Clustering {
 public:
  Clustering(const std::string &url);

  void parse(const std::string &url);

  inline const std::string &get_dataset() const {
    return _dataset;
  }

  inline uint32_t get_clusters() const {
    return _clusters;
  }

  inline uint32_t get_iterations() const {
    return _iterations;
  }

  inline const std::string &get_cluster_by() const {
    return _cluster_by;
  }

  inline const std::string &get_group_by() const {
    return _group_by;
  }

  std::string get_aggr_gaussian() const;

  std::string get_aggr_pdigest() const;

 protected:
  std::string _dataset;

  std::string _group_by;
  std::string _cluster_by;

  std::uint32_t _clusters{2};
  std::uint32_t _iterations{10};
  std::vector<std::string> _fields;
};

