/**
* Kalman filter implementation using Eigen. Based on the following
* introductory paper:
*
*     http://www.cs.unc.edu/~welch/media/pdf/kalman_intro.pdf
*
* @author: Hayk Martirosyan
* @date: 2014.11.15
 *  Copyright (c) 2019 Horizon Robotics. All rights reserved.
*/

#pragma once
#include <memory>
#include "Eigen/Dense"

namespace hobot {
namespace iou_mot {
class KalmanFilter {
 public:
  /**
  * Create a Kalman filter with the specified matrices.
  *   A - System dynamics matrix
  *   C - Output matrix
  *   Q - Process noise covariance
  *   R - Measurement noise covariance
  *   P - Estimate error covariance
  */
  KalmanFilter(
          double dt,
          const Eigen::MatrixXd &A,
          const Eigen::MatrixXd &C,
          const Eigen::MatrixXd &Q,
          const Eigen::MatrixXd &R,
          const Eigen::MatrixXd &P);

  /**
  * Create a blank estimator.
  */
  KalmanFilter();

  /**
  * Initialize the filter with initial states as zero.
  */
  void init();

  /**
  * Initialize the filter with a guess for initial states.
  */
  void init(double t0, const Eigen::VectorXd &x0);

  /**
  * Update the estimated state based on measured values. The
  * time step is assumed to remain constant.
  */
  void update(const Eigen::VectorXd &y);

  /**
  * Update the estimated state based on measured values,
  * using the given time step and dynamics matrix.
  */
  void update(const Eigen::VectorXd &y, double dt, const Eigen::MatrixXd A);

  void predict(int width, int height);

  /**
  * Return the current state and time.
  */
  Eigen::VectorXd state() {return x_hat;}

  int setState(int idx, float values);

  // Eigen::VectorXd predict() {return x_hat_new;}
  double time() {return t;}

 private:
  // Matrices for computation
  Eigen::MatrixXd A, C, Q, R, P, K, P0;

  // System dimensions
  int m, n;

  // Initial and current time
  double t0, t;

  // Discrete time step
  double dt;

  // Is the filter initialized?
  bool initialized;

  // n-size identity
  Eigen::MatrixXd I;

  // Estimated states
  Eigen::VectorXd x_hat, x_hat_new;
};

typedef std::shared_ptr<KalmanFilter> spKalmanFilter;
}  // namespace iou_mot
}  // namespace hobot
