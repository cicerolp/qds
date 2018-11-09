//
// Created by cicerolp on 4/17/18.
//

#pragma once

class Clustering {
 public:
  using aggr_expr = std::pair<std::string, std::vector<std::string>>;

  Clustering(const std::string &url);

  void parse(const std::string &url);

  inline const std::string &get_dataset() const {
    return _dataset;
  }

  inline uint32_t get_n_clusters() const {
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

  inline const std::string &get_group_by_clausule() const {
    return _group_by_clausule;
  }

  std::string get_aggr() const;

 protected:
  std::string _dataset;

  std::string _group_by;
  std::string _group_by_clausule;

  std::string _cluster_by;

  std::uint32_t _clusters{2};
  std::uint32_t _iterations{10};

  // [aggr type, [fields]]
  std::vector<aggr_expr> _aggr;
};

