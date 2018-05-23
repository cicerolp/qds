#include "stdafx.h"

#include "NDS.h"

#include "Categorical.h"
#include "Spatial.h"
#include "Temporal.h"

NDS::NDS(const Schema &schema) {
  std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
  start = std::chrono::high_resolution_clock::now();

  uint32_t pivots_count = 0;

  Data data(schema);

  std::cout << "Input Data: " << std::endl;
  for (auto &file : schema.files) {
    std::cout << "\tFile: " << file << std::endl;
  }
  std::cout << std::endl;

  std::cout << "NDS: " << std::endl;
  std::cout << "\tName: " << schema.name << std::endl;
  std::cout << "\tSize: " << data.size() << std::endl;
  std::cout << std::endl;

  _root = pivot_ctn(1);
  _root[0] = Pivot(0, data.size());

#ifdef NDS_ENABLE_PAYLOAD
  for (auto i = 0; i < schema.payload.size(); ++i) {
    const auto &info = schema.payload[i];

    switch (info.bin) {
      case 0: {
        std::cout << "\tPayload Dimension: PDigest\n\t\t" << info << std::endl;
        _payload.emplace_back(std::make_unique<PDigest>(info));
      }
        break;
      case 1: {
        std::cout << "\tPayload Dimension: Gaussian\n\t\t" << info << std::endl;
        _payload.emplace_back(std::make_unique<Gaussian>(info));
      }
        break;
    }

    // store payload index
    _payload_index[info.index] = i;

    // prepare payload
    data.preparePayload(info.offset);
  }
  std::cout << std::endl;

  // create root payload
  NDS::create_payload(data, _root[0]);
#endif // NDS_ENABLE_PAYLOAD

  BuildPair<link_ctn> links;
  links.input.emplace_back(&_root);

  BuildPair<build_ctn> range;
  range.input.emplace_back(0, data.size());

  for (auto i = 0; i < schema.dimension.size(); ++i) {
    auto &info = schema.dimension[i];

    switch (info.type) {
      case DimensionSchema::Spatial: {
        std::cout << "\tSpatial Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Spatial>(info));
      }
        break;
      case DimensionSchema::Temporal: {
        std::cout << "\tTemporal Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Temporal>(info));
      }
        break;
      case DimensionSchema::Categorical: {
        std::cout << "\tCategorical Dimension: \n\t\t" << info << std::endl;
        _dimension.emplace_back(std::make_unique<Categorical>(info));
      }
        break;
      default: {
        std::cerr << "error: invalid NDS" << std::endl;
        std::abort();
      }
        break;
    }

    uint32_t curr_count = _dimension.back()->build(*this, data, range, links);
    pivots_count += curr_count;

    swap_and_clear<link_ctn>(links.input, links.output);
    swap_and_clear<build_ctn>(range.input, range.output);

    std::cout << "\t\tNumber of Pivots: " + std::to_string(curr_count) << std::endl;

    // store payload index
    _dimension_index[info.index] = i;
  }

#ifdef NDS_ENABLE_PAYLOAD
  for (auto &elt : _payload) {
    elt->dispose_buffers();
  }
#endif // NDS_ENABLE_PAYLOAD

  std::cout << "\n\tTotal Number of Pivots: " << pivots_count << std::endl;

  end = std::chrono::high_resolution_clock::now();
  long long duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

  std::cout << "\tDuration: " + std::to_string(duration) + "s\n" << std::endl;
}

std::string NDS::query(const Query &query) {
  subset_ctn subsets;

  RangePivot root(_root[0]);

  if (!validation(query, subsets)) {
    root.pivot.back(0);
  }

  return serialize(query, subsets, root);
}

std::string NDS::pipeline(const Pipeline &pipeline) {
  subset_ctn source_ctn, dest_ctn;

  RangePivot root(_root[0]);

  if (!validation(pipeline.get_source(), source_ctn) || !validation(pipeline.get_dest(), dest_ctn)) {
    root.pivot.back(0);
  }

  return serialize_pipeline(pipeline, source_ctn, dest_ctn, root);
}

std::string get_values(std::vector<uint16_t> cluster) {
  std::string values;
  for (auto v: cluster) {
    values += std::to_string(v) + ":";
  }
  values = values.substr(0, values.size() - 1);

  return values;
}

std::vector<std::vector<uint16_t>> NDS::initialize_clusters(const Clustering &clustering, size_t n_objs) {
  std::vector<std::vector<uint16_t>> clusters;

  if (clustering.get_aggr().empty()) {
    return clusters;
  }

  /*std::vector<std::vector<uint16_t>> clusters(clustering.get_n_clusters());

  std::vector<uint16_t> values(n_objs);
  std::iota(values.begin(), values.end(), 0);

  // std::srand(unsigned(std::time(0)));
  std::srand(100);
  std::random_shuffle(values.begin(), values.end());

  // choose one object per centroid
  size_t obj = 0;
  for (auto &cluster : clusters) {
    cluster.emplace_back(values[obj++]);
  }*/

  /*std::random_device rd;
  *//*std::mt19937 mt_random(rd());*//*
  std::mt19937 mt_random(123);

  float total_distance = 0.f;

  std::vector<bool> objs_skip(n_objs, false);
  std::vector<float> objs_distance(n_objs, 0.f);

  // queries bases
  std::string right_base = "/destination/dataset=" +
      clustering.get_dataset() +
      clustering.get_aggr() +
      clustering.get_group_by_clausule();

  std::string left_base = "/source/dataset=" +
      clustering.get_dataset() +
      clustering.get_aggr() +
      clustering.get_group_by_clausule();

  // step 1 - choose first cluster center uniformly at random from data points
  auto initial_centroid = std::uniform_int_distribution<uint16_t>(0, n_objs - 1)(mt_random);
  *//*auto initial_centroid = 0;*//*

  clusters.push_back({initial_centroid});
  objs_skip[initial_centroid] = true;

  uint32_t selected_k = 1;

  // repeat steps 2 and 3 until k centers have been chosen
  while (selected_k != clustering.get_n_clusters()) {
    // reset total distance
    total_distance = 0.f;

    if (clustering.get_group_by_clausule().empty()) {
      std::vector<NDS::GroupBy<AggrSummarizeCtn>> clusters_summarize;

      for (auto &cluster : clusters) {
        clusters_summarize.emplace_back(get_cluster_summarize(
            Query(right_base + "/const=" + clustering.get_cluster_by() + ".values.(" + get_values(cluster) + ")")
        ));
      }

      for (uint16_t obj = 0; obj < n_objs; ++obj) {
        auto query = left_base + "/const=" + clustering.get_cluster_by() + ".values.(" + std::to_string(obj) + ")";

        auto values = get_raw_by_summarize(Query(query), clusters_summarize);

        // step 2 - for each obs x, compute distance d(x) to nearest cluster center
        auto distance = 1.f + (*std::min_element(std::begin(values), std::end(values)));

        objs_distance[obj] = distance * distance;
        total_distance += objs_distance[obj];
      }

    } else {
      std::vector<NDS::GroupBy<AggrGroupByCtn>> clusters_group_by;

      for (auto &cluster : clusters) {
        clusters_group_by.emplace_back(get_cluster_grop_by(
            Query(right_base + "/const=" + clustering.get_cluster_by() + ".values.(" + get_values(cluster) + ")")
        ));
      }

      for (uint16_t obj = 0; obj < n_objs; ++obj) {
        auto query = left_base + "/const=" + clustering.get_cluster_by() + ".values.(" + std::to_string(obj) + ")";

        auto values = get_raw_by_group(Query(query), clusters_group_by);

        // step 2 - for each obs x, compute distance d(x) to nearest cluster center
        auto distance = 1.f + (*std::min_element(std::begin(values), std::end(values)));

        objs_distance[obj] = distance * distance;
        total_distance += objs_distance[obj];
      }
    }

    // step 3 - choose new cluster center from amongst data points,
    // with probability of x being chosen proportional to d(x)^2

    auto random_distance = std::uniform_real_distribution<float>(0, total_distance)(mt_random);

    for (uint16_t obj = 0; obj < n_objs; ++obj) {
      if (!objs_skip[obj]) {
        random_distance -= objs_distance[obj];

        if (random_distance <= 0.0f) {
          // select this obj as centroid

          clusters.push_back({obj});
          objs_skip[obj] = true;

          selected_k++;

          // stop selecting the next centroid
          break;
        }
      }
    }
  }*/


  std::random_device rd;
  // std::mt19937 mt_random(rd());
  std::mt19937 mt_random(123);

  std::vector<float> objs_distance(n_objs, 0.f);

  // queries bases
  std::string right_base = "/destination/dataset=" +
      clustering.get_dataset() +
      clustering.get_aggr() +
      clustering.get_group_by_clausule();

  std::string left_base = "/source/dataset=" +
      clustering.get_dataset() +
      clustering.get_aggr() +
      clustering.get_group_by_clausule();

  // step 1 - choose first cluster center uniformly at random from data points
  auto initial_centroid = std::uniform_int_distribution<uint16_t>(0, n_objs - 1)(mt_random);
  /*auto initial_centroid = 0;*/

  clusters.push_back({initial_centroid});

  uint32_t selected_k = 1;

  // repeat steps 2 and 3 until k centers have been chosen
  while (selected_k != clustering.get_n_clusters()) {
    if (clustering.get_group_by_clausule().empty()) {
      std::vector<NDS::GroupBy<AggrSummarizeCtn>> clusters_summarize;

      for (auto &cluster : clusters) {
        clusters_summarize.emplace_back(get_cluster_summarize(
            Query(right_base + "/const=" + clustering.get_cluster_by() + ".values.(" + get_values(cluster) + ")")
        ));
      }

      for (uint16_t obj = 0; obj < n_objs; ++obj) {
        auto query = left_base + "/const=" + clustering.get_cluster_by() + ".values.(" + std::to_string(obj) + ")";
        auto values = get_raw_by_summarize(Query(query), clusters_summarize);

        // step 2 - for each obs x, compute distance d(x) to nearest cluster center
        objs_distance[obj] = (*std::min_element(std::begin(values), std::end(values)));
      }

    } else {
      std::vector<NDS::GroupBy<AggrGroupByCtn>> clusters_group_by;

      for (auto &cluster : clusters) {
        clusters_group_by.emplace_back(get_cluster_grop_by(
            Query(right_base + "/const=" + clustering.get_cluster_by() + ".values.(" + get_values(cluster) + ")")
        ));
      }
      for (uint16_t obj = 0; obj < n_objs; ++obj) {
        auto query = left_base + "/const=" + clustering.get_cluster_by() + ".values.(" + std::to_string(obj) + ")";
        auto values = get_raw_by_group(Query(query), clusters_group_by);

        // step 2 - for each obs x, compute distance d(x) to nearest cluster center
        objs_distance[obj] = (*std::min_element(std::begin(values), std::end(values)));
      }
    }

    // step 3 - choose new cluster center from amongst data points

    auto elt = std::max_element(objs_distance.begin(), objs_distance.end());
    uint16_t id = std::distance(objs_distance.begin(), elt);

    clusters.push_back({id});
    selected_k++;
  }

  return clusters;
}

NDS::GroupBy<AggrGroupByCtn> NDS::get_cluster_grop_by(const Query &query) const {
  subset_ctn ctn;

  // validadion of destionation
  validation(query, ctn);

  // range and subset from destination
  CopyOption option = DefaultCopy;

  auto range = get_range(ctn, option);
  auto subset = get_subset(ctn);

  auto aggr = get_aggr_group_by_ctn(query);
  do_group_by(aggr, range, subset, option);

  return GroupBy<AggrGroupByCtn>(aggr, range, subset);
}

std::vector<float> NDS::get_raw_by_group(const Query &query,
                                         const std::vector<NDS::GroupBy<AggrGroupByCtn>> &clusters) {
  auto value_group_by = get_cluster_grop_by(query);

  std::vector<float> raw;
  std::vector<float> values;

  for (auto &cluster : clusters) {
    auto groups = std::make_pair(value_group_by, cluster);

    // clear buffers
    raw.clear();

    right_join_equal(groups, raw);
    // left_join_equal(groups, raw);

    auto accum = std::accumulate(raw.begin(), raw.end(), 0.f);
    values.emplace_back(accum / raw.size());
  }

  return values;
}

NDS::GroupBy<AggrSummarizeCtn> NDS::get_cluster_summarize(const Query &query) const {
  subset_ctn ctn;

  // validadion of destionation
  validation(query, ctn);

  // range and subset from destination
  CopyOption option = DefaultCopy;

  auto range = get_range(ctn, option);
  auto subset = get_subset(ctn);

  auto aggr = get_aggr_summarize_ctn(query);
  do_summarize(aggr, range, subset);

  return GroupBy<AggrSummarizeCtn>(aggr, range, subset);
}

std::vector<float> NDS::get_raw_by_summarize(const Query &query,
                                             const std::vector<NDS::GroupBy<AggrSummarizeCtn>> &clusters) {
  auto value_summarize = get_cluster_summarize(query);

  std::vector<float> raw;
  std::vector<float> values;

  for (auto &cluster : clusters) {
    auto groups = std::make_pair(value_summarize, cluster);

    // clear buffers
    raw.clear();

    summarize_equal(groups, raw);

    auto accum = std::accumulate(raw.begin(), raw.end(), 0.f);
    values.emplace_back(accum / raw.size());
  }

  return values;
}

std::string NDS::clustering(const Clustering &clustering) {
#ifdef ENABLE_GPERF
  ProfilerStart("perf.prof");
#endif

  float threshold = 0.005;

  auto n_objs = _dimension[_dimension_index[clustering.get_cluster_by()]]->get_schema().bin;
  auto clusters = initialize_clusters(clustering, n_objs);

  std::vector<int32_t> clusters_size(clustering.get_n_clusters(), 0);

  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();
  writer.StartArray();

  // queries bases
  std::string right_base = "/destination/dataset=" +
      clustering.get_dataset() +
      clustering.get_aggr() +
      clustering.get_group_by_clausule();

  std::string left_base = "/source/dataset=" +
      clustering.get_dataset() +
      clustering.get_aggr() +
      clustering.get_group_by_clausule();

  // assignment step
  auto iterations = clustering.get_iterations();
  while (iterations--) {
    // keep further trajectory to repopulate empty clusters
    size_t further_cluster;
    size_t further_trajectory;
    float further_distance = std::numeric_limits<float>::min();

    if (clustering.get_group_by_clausule().empty()) {
      std::vector<NDS::GroupBy<AggrSummarizeCtn>> clusters_summarize;

      for (auto &cluster : clusters) {
        clusters_summarize.emplace_back(get_cluster_summarize(
            Query(right_base + "/const=" + clustering.get_cluster_by() + ".values.(" + get_values(cluster) + ")")
        ));

        // clear cluster
        cluster.clear();
      }

      for (auto obj = 0; obj < n_objs; ++obj) {
        auto query = left_base + "/const=" + clustering.get_cluster_by() + ".values.(" + std::to_string(obj) + ")";

        auto values = get_raw_by_summarize(Query(query), clusters_summarize);
        auto distance = std::min_element(std::begin(values), std::end(values));

        auto centroid = std::distance(std::begin(values), distance);

        // update step
        clusters[centroid].emplace_back(obj);

        // keep further trajectory
        if ((*distance) > further_distance) {
          further_cluster = centroid;
          further_trajectory = clusters[centroid].size() - 1;
          further_distance = (*distance);
        }
      }

    } else {
      std::vector<NDS::GroupBy<AggrGroupByCtn>> clusters_group_by;

      for (auto &cluster : clusters) {
        clusters_group_by.emplace_back(get_cluster_grop_by(
            Query(right_base + "/const=" + clustering.get_cluster_by() + ".values.(" + get_values(cluster) + ")")
        ));

        // clear cluster
        cluster.clear();
      }

      for (auto obj = 0; obj < n_objs; ++obj) {
        auto query = left_base + "/const=" + clustering.get_cluster_by() + ".values.(" + std::to_string(obj) + ")";

        auto values = get_raw_by_group(Query(query), clusters_group_by);
        auto distance = std::min_element(std::begin(values), std::end(values));

        auto centroid = std::distance(std::begin(values), distance);

        // update step
        clusters[centroid].emplace_back(obj);

        // keep further trajectory
        if ((*distance) > further_distance) {
          further_cluster = centroid;
          further_trajectory = clusters[centroid].size() - 1;
          further_distance = (*distance);
        }
      }
    }

    bool empty_cluster = false;

    // repopulate (one) empty cluster
    for (auto &cluster: clusters) {
      if (cluster.empty()) {
        empty_cluster = true;
        std::cout << "[debug] repopulate empty cluster with: #" << further_cluster << std::endl;

        // push into empty cluster
        cluster.emplace_back(clusters[further_cluster][further_trajectory]);

        // pop further trajectory
        clusters[further_cluster].erase(clusters[further_cluster].begin() + further_trajectory);
        break;
      }
    }

    std::cout << "[debug] ::";
    for (auto &cluster: clusters) {
      std::cout << cluster.size() << "::";
    }
    std::cout << std::endl;

    if (!empty_cluster) {
      size_t diff = 0;
      for (auto i = 0; i < clusters.size(); ++i) {
        diff += std::abs((int32_t) clusters[i].size() - clusters_size[i]);

        clusters_size[i] = clusters[i].size();
      }

      if (diff / (float) n_objs <= threshold) {
        std::cout << "[debug] diff below threshold: " << diff / (float) n_objs << " <= " << threshold << std::endl;
        break;
      } else {
        std::cout << "[debug] diff above threshold: " << diff / (float) n_objs << " > " << threshold << std::endl;
      }
    }
  }

  std::cout << "[debug] # iterations: " << clustering.get_iterations() - iterations << std::endl;

  // output json
  for (auto &cluster : clusters) {
    writer.StartArray();

    for (auto &value : cluster) {
      writer.Uint(value);
    }
    writer.EndArray();
  }

  // end json
  writer.EndArray();
  writer.EndArray();
  auto output = buffer.GetString();

#ifdef ENABLE_GPERF
  ProfilerStop();
#endif

  return output;
}

std::string NDS::augmented_series(const AugmentedSeries &augmented_series) {
#ifdef ENABLE_GPERF
  ProfilerStart("perf.prof");
#endif

  // raw data buffer
  std::vector<float> raw;

  // get pipeline from augmented_series
  auto pipeline = augmented_series.get_pipeline();
  auto source_query = Query(pipeline.get_source());
  auto &bounds = augmented_series.get_bounds();
  auto &dimension = augmented_series.get_dimension();

  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();
  writer.StartArray();

  subset_ctn source_ctn, dest_ctn;

  RangePivot root(_root[0]);

  // validadion of destionation
  validation(pipeline.get_dest(), dest_ctn);

  // range and subset from destination
  CopyOption option_dest = DefaultCopy;
  auto range_dest = get_range(dest_ctn, option_dest);
  auto subset_dest = get_subset(dest_ctn);

  // summarize
  if (!pipeline.get_source().group_by()) {

    auto aggr_dest = get_aggr_summarize_ctn(pipeline.get_dest());
    do_summarize(aggr_dest, range_dest, subset_dest);

    for (const auto &interval : bounds) {
      source_query.parse("/const=" + dimension + ".interval.(" + interval.to_string() + ")");

      // validation of source
      source_ctn.clear();
      validation(source_query, source_ctn);

      CopyOption option_source = DefaultCopy;
      auto range_source = get_range(source_ctn, option_source);
      auto subset_source = get_subset(source_ctn);

      auto aggr_source = get_aggr_summarize_ctn(pipeline.get_source());
      do_summarize(aggr_source, range_source, subset_source);

      auto groups = std::make_pair(GroupBy<AggrSummarizeCtn>(aggr_source, range_source, subset_source),
                                   GroupBy<AggrSummarizeCtn>(aggr_dest, range_dest, subset_dest));

      raw.clear();
      summarize_pipe(groups, raw);

      double accum = 0;
      for (auto &elt : raw) {
        if (elt >= 0) {
          accum += std::fabs(elt - 0.5);
        }
      }
      writer.StartArray();
      writer.Uint(interval.bound[0]);
      writer.Double(accum);
      writer.EndArray();
    }
  } else {
    auto aggr_dest = get_aggr_group_by_ctn(pipeline.get_dest());
    do_group_by(aggr_dest, range_dest, subset_dest, option_dest);

    for (const auto &interval : bounds) {
      source_query.parse("/const=" + dimension + ".interval.(" + interval.to_string() + ")");

      // validation of source
      source_ctn.clear();
      validation(source_query, source_ctn);

      CopyOption option_source = DefaultCopy;
      auto range_source = get_range(source_ctn, option_source);
      auto subset_source = get_subset(source_ctn);

      auto aggr_source = get_aggr_group_by_ctn(pipeline.get_source());
      do_group_by(aggr_source, range_source, subset_source, option_source);

      auto groups = std::make_pair(GroupBy<AggrGroupByCtn>(aggr_source, range_source, subset_source),
                                   GroupBy<AggrGroupByCtn>(aggr_dest, range_dest, subset_dest));

      raw.clear();
      if (pipeline.get_join() == "inner_join") {
        group_by_inner_join(groups, raw, pipeline.get_threshold());
      } else if (pipeline.get_join() == "left_join") {
        group_by_left_join(groups, raw, pipeline.get_threshold());
      } else if (pipeline.get_join() == "right_join") {
        group_by_right_join(groups, raw, pipeline.get_threshold());
      }

      double accum = 0;
      for (auto &elt : raw) {
        if (elt >= 0) {
          accum += std::fabs(elt - 0.5);
        }
      }
      writer.StartArray();
      writer.Uint(interval.bound[0]);
      writer.Double(accum);
      writer.EndArray();
    }
  }

  // end json
  writer.EndArray();
  writer.EndArray();
  auto output = buffer.GetString();

#ifdef ENABLE_GPERF
  ProfilerStop();
#endif

  return output;
}

std::string NDS::serialize(const Query &query, subset_ctn &subsets, const RangePivot &root) const {
  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();

  if (root.pivot.empty()) {
    // empty response
    writer.StartArray();
    writer.EndArray();

  } else {
    CopyOption option = DefaultCopy;

    auto range = get_range(subsets, option);
    auto subset = get_subset(subsets);

    // summarize
    if (!query.group_by()) {
      auto aggr = get_aggr_summarize_ctn(query);
      do_summarize(aggr, range, subset);

      summarize_query(aggr, writer);

    } else {
      auto aggr = get_aggr_group_by_ctn(query);
      do_group_by(aggr, range, subset, option);

      group_by_query(aggr, writer);
    }
  }

  // end json
  writer.EndArray();
  return buffer.GetString();
}

std::string NDS::serialize_pipeline(const Pipeline &pipeline,
                                    const subset_ctn &source_ctn,
                                    const subset_ctn &dest_ctn,
                                    const RangePivot &root) const {
  // serialization
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  // start json
  writer.StartArray();

  if (root.pivot.empty()) {
    // empty response
    writer.StartArray();
    writer.EndArray();

  } else if (pipeline.get_source().get_group_by() != pipeline.get_dest().get_group_by()) {
    // invalid pipeline
    writer.StartArray();
    writer.EndArray();

  } else {
    CopyOption option_source = DefaultCopy;
    auto range_source = get_range(source_ctn, option_source);
    auto subset_source = get_subset(source_ctn);

    CopyOption option_dest = DefaultCopy;
    auto range_dest = get_range(dest_ctn, option_dest);
    auto subset_dest = get_subset(dest_ctn);

    // summarize
    if (!pipeline.get_source().group_by()) {
      auto aggr_source = get_aggr_summarize_ctn(pipeline.get_source());
      do_summarize(aggr_source, range_source, subset_source);

      auto aggr_dest = get_aggr_summarize_ctn(pipeline.get_dest());
      do_summarize(aggr_dest, range_dest, subset_dest);

      auto groups = std::make_pair(GroupBy<AggrSummarizeCtn>(aggr_source, range_source, subset_source),
                                   GroupBy<AggrSummarizeCtn>(aggr_dest, range_dest, subset_dest));

      summarize_pipe(groups, writer);

    } else {
      auto aggr_source = get_aggr_group_by_ctn(pipeline.get_source());
      do_group_by(aggr_source, range_source, subset_source, option_source);

      auto aggr_dest = get_aggr_group_by_ctn(pipeline.get_dest());
      do_group_by(aggr_dest, range_dest, subset_dest, option_dest);

      auto groups = std::make_pair(GroupBy<AggrGroupByCtn>(aggr_source, range_source, subset_source),
                                   GroupBy<AggrGroupByCtn>(aggr_dest, range_dest, subset_dest));

      if (pipeline.get_join() == "inner_join") {
        group_by_inner_join(groups, writer, pipeline.get_threshold());
      } else if (pipeline.get_join() == "left_join") {
        group_by_left_join(groups, writer, pipeline.get_threshold());
      } else if (pipeline.get_join() == "right_join") {
        group_by_right_join(groups, writer, pipeline.get_threshold());
      }
    }
  }

  // end json
  writer.EndArray();
  return buffer.GetString();
}

range_ctn NDS::get_range(const subset_ctn &subsets, CopyOption &option) const {
  if (subsets.size() == 0) {
    return {_root[0]};
  } else {
    range_ctn range, response{_root[0]};

    for (auto i = 0; i < subsets.size() - 1; ++i) {
      restrict(range, response, subsets[i], option);
    }

    if (option == DefaultCopy) option = subsets.back().option;

    // sort range only when necessary
    swap_and_sort(range, response, option);

    return range;
  }
}

bined_ctn NDS::get_subset(const subset_ctn &subsets) const {
  if (subsets.size() == 0) {
    return bined_ctn();
  } else {
    return subsets.back().container;
  }
}

void NDS::restrict(range_ctn &range, range_ctn &response, const subset_t &subset, CopyOption &option) const {
  if (option == DefaultCopy) option = subset.option;

  // sort range only when necessary
  swap_and_sort(range, response, option);

  pivot_it it_lower, it_upper;
  range_it it_range;

  switch (option) {
    case CopyValueFromRange:
      for (const auto &el : subset.container) {
        it_lower = el->ptr().begin();
        it_range = range.begin();
        while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
          while (it_lower != it_upper) {
            response.emplace_back((*it_lower++), (*it_range).value);
          }
          ++it_range;
        }
      }
      break;
    case CopyValueFromSubset:
      for (const auto &el : subset.container) {
        it_lower = el->ptr().begin();
        it_range = range.begin();
        while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
          while (it_lower != it_upper) {
            response.emplace_back((*it_lower++), el->value);
          }
          ++it_range;
        }
      }
      break;
    default:
      for (const auto &el : subset.container) {
        it_lower = el->ptr().begin();
        it_range = range.begin();
        while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
          response.insert(response.end(), it_lower, it_upper);
          it_lower = it_upper;
          ++it_range;
        }
      }
      break;
  }

  if (option == CopyValueFromSubset) option = CopyValueFromRange;
}

bool NDS::search_iterators(range_it &it_range, const range_ctn &range,
                           pivot_it &it_lower, pivot_it &it_upper, const pivot_ctn &subset) const {
  if (it_lower == subset.end() || it_range == range.end()) return false;

  if ((*it_range).pivot.ends_before(*it_lower)) {
    // binary search to find range iterator
    it_range = std::lower_bound(it_range, range.end(), (*it_lower), RangePivot::upper_bound_comp);

    if (it_range == range.end()) return false;
  }

  if ((*it_range).pivot.begins_after(*it_lower)) {
    if (it_lower == std::prev(subset.end())) return false;

    // binnary search to find lower subset iterator
    it_lower = std::lower_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::lower_bound_comp);

    if (it_lower == subset.end()) return false;
  }

  it_upper = std::upper_bound(it_lower, subset.end(), (*it_range).pivot, Pivot::upper_bound_comp);

  return true;
}

void NDS::compactation(range_ctn &input, range_ctn &output, CopyOption option) const {
  output.emplace_back(input.front());

  if (option == DefaultCopy || option == CopyValueFromSubset) {
    for (size_t i = 1; i < input.size(); ++i) {
      if (Pivot::is_sequence(output.back().pivot, input[i].pivot)) {
        output.back().pivot.append(input[i].pivot);
      } else {
        output.emplace_back(input[i]);
      }
    }
  } else { // CopyValueFromRange
    for (size_t i = 1; i < input.size(); ++i) {
      if (RangePivot::is_sequence(output.back(), input[i])) {
        output.back().pivot.append(input[i].pivot);
      } else {
        output.emplace_back(input[i]);
      }
    }
  }
}

std::string NDS::schema() const {

  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartObject();

  writer.Key("count");
  writer.Int(_root[0].size());

  writer.Key("index_dimensions");
  writer.StartArray();
  for (const auto &ptr : _dimension) {
    writer.StartObject();
    ptr->get_schema(writer);
    writer.EndObject();
  }
  writer.EndArray();

  writer.Key("index_payloads");
  writer.StartArray();
  for (const auto &ptr : _payload) {
    writer.StartObject();
    ptr->get_schema(writer);
    writer.EndObject();
  }
  writer.EndArray();

  writer.EndObject();

  return buffer.GetString();
}

void NDS::summarize_query(const AggrSummarizeCtn &aggrs, json &writer) const {
  for (auto &aggr : aggrs) {
    writer.StartArray();
    aggr->output(writer);
    writer.EndArray();
  }
}

void NDS::group_by_query(const AggrGroupByCtn &aggrs, json &writer) const {
  for (auto &aggr : aggrs) {
    writer.StartArray();
    aggr->output(writer);
    writer.EndArray();
  }
}

void NDS::group_by_data(const AggrGroupByCtn &aggrs, json &writer) const {
  std::vector<uint64_t> keys;

  for (auto &aggr : aggrs) {
    aggr->insert_mapped_keys(keys);

    writer.StartArray();
    for (auto &value: keys) {
      writer.Uint64(value);
    }
    writer.EndArray();
  }
}
void NDS::group_by_data(const AggrGroupByCtn &aggrs, std::vector<uint64_t> &raw) const {
  std::vector<uint64_t> keys;

  for (auto &aggr : aggrs) {
    aggr->insert_mapped_keys(keys);
    for (auto &value: keys) {
      raw.emplace_back(value);
    }
  }
}

void NDS::group_by_inner_join(const GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const {
  std::vector<uint64_t> keys;

  for (auto &source_aggr : groups.first.aggrs) {
    writer.StartArray();
    // get keys from source
    source_aggr->insert_mapped_keys(keys, threshold);

    for (auto &key : keys) {
      // get pipe from source
      auto pipe = source_aggr->get_pipe(key, threshold);
      for (auto &dest_aggr : groups.second.aggrs) {
        dest_aggr->output_two_way(key, pipe, writer, threshold);
      }
    }
    writer.EndArray();
  }
}

void NDS::group_by_left_join(const GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const {
  std::vector<uint64_t> keys;

  for (auto &source_aggr : groups.first.aggrs) {
    writer.StartArray();
    // get keys from source
    source_aggr->insert_mapped_keys(keys);

    for (auto &key : keys) {
      // get pipe from sorce
      auto pipe = source_aggr->get_pipe(key, threshold);
      for (auto &dest_aggr : groups.second.aggrs) {
        dest_aggr->output_one_way(key, pipe, writer);
      }
    }
    writer.EndArray();
  }
}

void NDS::group_by_right_join(const GroupCtn<AggrGroupByCtn> &groups, json &writer, uint32_t threshold) const {
  std::vector<uint64_t> keys;

  for (auto &source_aggr : groups.first.aggrs) {
    writer.StartArray();
    for (auto &dest_aggr : groups.second.aggrs) {
      // get keys from destination
      dest_aggr->insert_mapped_keys(keys, threshold);

      for (auto &key : keys) {
        // get pipe from source
        auto pipe = source_aggr->get_pipe(key, 0);
        dest_aggr->output_one_way(key, pipe, writer);
      }
    }
    writer.EndArray();
  }
}

void NDS::group_by_inner_join(const NDS::GroupCtn<std::vector<std::shared_ptr<AggrGroupBy>>> &groups,
                              std::vector<float> &raw,
                              uint32_t threshold) const {
  std::vector<uint64_t> keys;

  for (auto &source_aggr : groups.first.aggrs) {
    // get keys from source
    source_aggr->insert_mapped_keys(keys, threshold);

    for (auto &key : keys) {
      // get pipe from source
      auto pipe = source_aggr->get_pipe(key, threshold);
      for (auto &dest_aggr : groups.second.aggrs) {
        dest_aggr->output_two_way(key, pipe, raw, threshold);
      }
    }
  }
}

void NDS::group_by_left_join(const NDS::GroupCtn<std::vector<std::shared_ptr<AggrGroupBy>>> &groups,
                             std::vector<float> &raw,
                             uint32_t threshold) const {
  std::vector<uint64_t> keys;

  for (auto &source_aggr : groups.first.aggrs) {
    // get keys from source
    source_aggr->insert_mapped_keys(keys);

    for (auto &key : keys) {
      // get pipe from sorce
      auto pipe = source_aggr->get_pipe(key, threshold);
      for (auto &dest_aggr : groups.second.aggrs) {
        dest_aggr->output_one_way(key, pipe, raw);
      }
    }
  }
}

void NDS::group_by_right_join(const GroupCtn<AggrGroupByCtn> &groups,
                              std::vector<float> &raw,
                              uint32_t threshold) const {
  std::vector<uint64_t> keys;

  for (auto &source_aggr : groups.first.aggrs) {
    for (auto &dest_aggr : groups.second.aggrs) {
      // get keys from destination
      dest_aggr->insert_mapped_keys(keys, threshold);

      for (auto &key : keys) {
        // get pipe from source
        auto pipe = source_aggr->get_pipe(key, 0);
        dest_aggr->output_one_way(key, pipe, raw);
      }
    }
  }
}

void NDS::summarize_equal(const GroupCtn<AggrSummarizeCtn> &groups, std::vector<float> &raw) const {
  if (groups.first.aggrs.size() == groups.second.aggrs.size()) {
    auto length = groups.first.aggrs.size();
    for (auto aggr = 0; aggr < length; ++aggr) {
      auto payload = groups.first.aggrs[aggr]->get_payload();
      groups.second.aggrs[aggr]->equality(payload, raw);
    }
  }
}

void NDS::left_join_equal(const GroupCtn<AggrGroupByCtn> &groups, std::vector<float> &raw) const {
  if (groups.first.aggrs.size() == groups.second.aggrs.size()) {
    std::vector<uint64_t> keys;

    auto length = groups.first.aggrs.size();
    for (auto aggr = 0; aggr < length; ++aggr) {
      // get keys from source
      groups.first.aggrs[aggr]->insert_mapped_keys(keys);

      for (auto &key: keys) {
        // get payload from sorce
        auto payload = groups.first.aggrs[aggr]->get_payload(key);
        // left join
        groups.second.aggrs[aggr]->equality_one_way(key, payload, raw);
      }
    }
  }
}

void NDS::right_join_equal(const GroupCtn<AggrGroupByCtn> &groups, std::vector<float> &raw) const {
  if (groups.first.aggrs.size() == groups.second.aggrs.size()) {
    std::vector<uint64_t> keys;

    auto length = groups.first.aggrs.size();
    for (auto aggr = 0; aggr < length; ++aggr) {
      // get keys from destination
      groups.second.aggrs[aggr]->insert_mapped_keys(keys);

      for (auto &key: keys) {
        // get payload from sorce
        auto payload = groups.first.aggrs[aggr]->get_payload(key);
        // right join
        groups.second.aggrs[aggr]->equality_one_way(key, payload, raw);
      }
    }
  }
}

void NDS::summarize_pipe(const GroupCtn<AggrSummarizeCtn> &groups, json &writer) const {
  for (auto &source_aggr : groups.first.aggrs) {
    auto pipe = source_aggr->get_pipe();

    writer.StartArray();
    for (auto &dest_aggr : groups.second.aggrs) {
      writer.StartArray();
      dest_aggr->output(pipe, writer);
      writer.EndArray();
    }
    writer.EndArray();
  }
}

void NDS::summarize_pipe(const GroupCtn<AggrSummarizeCtn> &groups, std::vector<float> &raw) const {
  for (auto &source_aggr : groups.first.aggrs) {
    auto pipe = source_aggr->get_pipe();

    for (auto &dest_aggr : groups.second.aggrs) {
      dest_aggr->output(pipe, raw);
    }
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// common methods
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NDS::do_summarize(AggrSummarizeCtn &aggrs, const range_ctn &range, const bined_ctn &subset) const {
  if (subset.size() != 0) {
    for (const auto &el : subset) {
      pivot_it it_lower = el->ptr().begin(), it_upper;
      range_it it_range = range.begin();

      while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
        if (it_lower != it_upper) {
          for (auto &aggr : aggrs) {
            aggr->merge(it_lower, it_upper);
          }
          it_lower = it_upper;
        }
        ++it_range;
      }
    }
  } else {
    for (auto &aggr : aggrs) {
      aggr->merge(range.begin(), range.end());
    }
  }
}

void NDS::do_group_by(AggrGroupByCtn &aggrs,
                      const range_ctn &range,
                      const bined_ctn &subset,
                      const CopyOption &option) const {
  if (option == CopyValueFromSubset) {
    for (auto el = 0; el < subset.size(); ++el) {
      pivot_it it_lower = subset[el]->ptr().begin(), it_upper;
      range_it it_range = range.begin();

      while (search_iterators(it_range, range, it_lower, it_upper, subset[el]->ptr())) {
        if (it_lower != it_upper) {
          for (auto &aggr : aggrs) {
            aggr->merge(subset[el]->value, it_lower, it_upper);
          }
          it_lower = it_upper;
        }
        ++it_range;
      }
    }
  } else {
    for (const auto &el : subset) {
      pivot_it it_lower = el->ptr().begin(), it_upper;
      range_it it_range = range.begin();

      while (search_iterators(it_range, range, it_lower, it_upper, el->ptr())) {
        if (it_lower != it_upper) {
          for (auto &aggr : aggrs) {
            aggr->merge((*it_range).value, it_lower, it_upper);
          }
          it_lower = it_upper;
        }
        ++it_range;
      }
    }
  }
}

AggrSummarizeCtn NDS::get_aggr_summarize_ctn(const Query &query) const {
  std::vector<std::shared_ptr<AggrSummarize>> aggr_ctn;

  // get aggregation clausule
  auto &aggr = query.get_aggr();

  for (const auto &expr : aggr) {
    if (expr.first == "count") {
      aggr_ctn.emplace_back(std::make_shared<AggrCountSummarize>(expr, get_payload_index(expr.second)));
    }
#ifdef ENABLE_PDIGEST
    else if (expr.first == "quantile" ||
        expr.first == "inverse" ||
        expr.first == "sector" ||
        expr.first == "ks" ||
        expr.first == "ksw") {
      aggr_ctn.emplace_back(std::make_shared<AggrPDigestSummarize>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
    else if (expr.first == "variance" || expr.first == "average") {
      aggr_ctn.emplace_back(std::make_shared<AggrGaussianSummarize>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_GAUSSIAN
  }

  return aggr_ctn;
}

AggrGroupByCtn NDS::get_aggr_group_by_ctn(const Query &query) const {
  std::vector<std::shared_ptr<AggrGroupBy>> aggr_ctn;

  // get aggregation clausule
  auto &aggr = query.get_aggr();

  for (const auto &expr : aggr) {
    if (expr.first == "count") {
      aggr_ctn.emplace_back(std::make_shared<AggrCountGroupBy>(expr, get_payload_index(expr.second)));
    }

#ifdef ENABLE_PDIGEST
    else if (expr.first == "quantile" ||
        expr.first == "inverse" ||
        expr.first == "sector" ||
        expr.first == "ks" ||
        expr.first == "ksw") {
      aggr_ctn.emplace_back(std::make_shared<AggrPDigestGroupBy>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_PDIGEST

#ifdef ENABLE_GAUSSIAN
    else if (expr.first == "variance" || expr.first == "average") {
      aggr_ctn.emplace_back(std::make_shared<AggrGaussianGroupBy>(expr, get_payload_index(expr.second)));
    }
#endif // ENABLE_GAUSSIAN
  }

  return aggr_ctn;
}
