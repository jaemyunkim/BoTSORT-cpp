#pragma once

#include "GlobalMotionCompensation.h"
#include "track.h"

#include <string>

/**
 * @brief Struct representing a detection
 * 
 * cv::Rect_<float> bbox_tlwh: Bounding box of the detection in the format (top left x, top left y, width, height)
 * int class_id: Class ID of the detection
 * float confidence: Confidence score of the detection
 */
struct Detection {
    cv::Rect_<float> bbox_tlwh;
    int class_id;
    float confidence;
};

class BoTSORT {
public:
    /**
     * @brief Execute the tracking algorithm on the given detections and frame
     * 
     * @param detections Detections (respresented using the Detection struct) in the current frame
     * @param frame Current frame
     * @return std::vector<Track>
     */
    std::vector<Track> track(const std::vector<Detection> &detections, const cv::Mat &frame);

private:
    GlobalMotionCompensation _gmc_algo;

    uint8_t _track_buffer, _frame_rate, _frame_id, _buffer_size, _max_time_lost;
    float _track_high_thresh, _new_track_thresh, _match_thresh, _proximity_thresh, _appearance_thresh, _lambda;

    bool _fp16_inference;

    std::vector<Track *> _tracked_tracks;
    std::vector<Track *> _lost_tracks;
    std::vector<Track *> _removed_tracks;

    byte_kalman::KalmanFilter _kalman_filter;


public:
    BoTSORT(
            std::optional<const char *> model_weights = "",
            bool fp16_inference = false,
            float track_high_thresh = 0.45,
            float new_track_thresh = 0.6,
            uint8_t track_buffer = 30,
            float match_thresh = 0.8,
            float proximity_thresh = 0.5,
            float appearance_thresh = 0.25,
            const char *gmc_method = "sparseOptFlow",
            uint8_t frame_rate = 30,
            float lambda = 0.985);
    ~BoTSORT();

private:
    /**
     * @brief Extract visual features from the given bounding box using CNN
     * 
     * @param frame Image frame
     * @param bbox_tlwh Bounding box in the format (top left x, top left y, width, height)
     * @return FeatureVector Extracted visual features
     */
    FeatureVector _extract_features(const cv::Mat &frame, const cv::Rect_<float> &bbox_tlwh);

    /**
     * @brief Merge two track lists into one with no duplicates
     * 
     * @param tracks_list_a First track list
     * @param tracks_list_b Second track list
     * @return std::vector<Track *> Merged track list
     */
    std::vector<Track *> _merge_track_lists(std::vector<Track *> &tracks_list_a, std::vector<Track *> &tracks_list_b);

    std::vector<Track> _sub_tracks(std::vector<Track> &tracks_list_a, std::vector<Track> &tracks_list_b);
    void _remove_duplicate_tracks(
            std::vector<Track> &result_tracks_a,
            std::vector<Track> &result_tracks_b,
            std::vector<Track> &tracks_list_a,
            std::vector<Track> &tracks_list_b);
};