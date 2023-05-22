#pragma once

#include "DataType.h"
#include "track.h"

/**
 * @brief Calculate the IoU distance between tracks and detections and create a mask for the cost matrix
 *  when the IoU distance is greater than the threshold
 * 
 * @param tracks Tracks used to create the cost matrix
 * @param detections Tracks created from detections used to create the cost matrix
 * @param max_iou_distance Threshold for IoU distance
 * @param iou_dists_mask Mask for the cost matrix (would be modified in-place)
 * @return CostMatrix IoU distance cost matrix
 */
CostMatrix iou_distance(const std::vector<std::shared_ptr<Track>> &tracks,
                        const std::vector<std::shared_ptr<Track>> &detections,
                        float max_iou_distance,
                        CostMatrix &iou_dists_mask);

/**
 * @brief Calculate the IoU distance between tracks and detections
 * 
 * @param tracks Tracks used to create the cost matrix
 * @param detections Tracks created from detections used to create the cost matrix
 * @return CostMatrix IoU distance cost matrix
 */
CostMatrix iou_distance(const std::vector<std::shared_ptr<Track>> &tracks,
                        const std::vector<std::shared_ptr<Track>> &detections);


/**
 * @brief Calculate the embedding distance between tracks and detections and create a mask for the cost matrix
 * 
 * @param tracks Tracks used to create the cost matrix
 * @param detections Tracks created from detections used to create the cost matrix
 * @param max_embedding_distance Threshold for embedding distance
 * @param embedding_dists_mask Mask for the cost matrix (would be modified in-place)
 * @return CostMatrix Embedding distance cost matrix
 */
CostMatrix embedding_distance(const std::vector<std::shared_ptr<Track>> &tracks,
                              const std::vector<std::shared_ptr<Track>> &detections,
                              float max_embedding_distance,
                              CostMatrix &embedding_dists_mask);

/**
 * @brief Fuses the detection score into the cost matrix in-place
 *     fused_cost = 1 - ((1 - cost_matrix) * detection_score)
 *     fused_cost = 1 - (similarity * detection_score)
 * 
 * @param cost_matrix Cost matrix in which to fuse the detection score
 * @param detections Tracks created from detections used to create the cost matrix
 */
void fuse_score(CostMatrix &cost_matrix, const std::vector<std::shared_ptr<Track>> &detections);

/**
 * @brief Fuses motion (maha distance) into the cost matrix in-place
 *      fused_cost = lambda * cost_matrix + (1 - lambda) * motion_distance
 * @param KF Kalman filter
 * @param cost_matrix Cost matrix in which to fuse motion
 * @param tracks Tracks used to create the cost matrix
 * @param detections Tracks created from detections used to create the cost matrix
 * @param lambda Weighting factor for motion (default: 0.98)
 * @param only_position Set to true only position should be used for gating distance
 */
void fuse_motion(const KalmanFilter &KF,
                 CostMatrix &cost_matrix,
                 const std::vector<std::shared_ptr<Track>> &tracks,
                 const std::vector<std::shared_ptr<Track>> &detections,
                 float lambda = 0.98F,
                 bool only_position = false);

/**
 * @brief Fuse IoU distance with embedding distance keeping the mask in mind
 * 
 * @param iou_dist Score fused IoU distance cost matrix
 * @param emb_dist Motion fused embedding distance cost matrix
 * @param iou_dists_mask IoU distance mask
 * @param emb_dists_mask Embedding distance mask
 * @return CostMatrix Fused and masked cost matrix
 */
CostMatrix fuse_iou_with_emb(CostMatrix &iou_dist,
                             CostMatrix &emb_dist,
                             const CostMatrix &iou_dists_mask,
                             const CostMatrix &emb_dists_mask);

/**
 * @brief Performs linear assignment using the LAPJV algorithm
 * 
 * @param cost_matrix Cost matrix
 * @param thresh Threshold for cost matrix
 * @param associations Output associations between tracks and detections
 */
void linear_assignment(CostMatrix &cost_matrix, float thresh, AssociationData &associations);