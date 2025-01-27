/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2013, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

/* Author: Sachin Chitta */

#pragma once

#include <rclcpp/rclcpp.hpp>
#include <moveit/macros/class_forward.hpp>
#include <moveit/planning_scene/planning_scene.hpp>
#include <object_recognition_msgs/msg/table_array.hpp>
#include <moveit_msgs/msg/collision_object.hpp>
#include <mutex>

namespace shapes
{
MOVEIT_CLASS_FORWARD(Shape);  // Defines ShapePtr, ConstPtr, WeakPtr... etc
}

namespace moveit
{
namespace semantic_world
{
MOVEIT_CLASS_FORWARD(SemanticWorld);  // Defines SemanticWorldPtr, ConstPtr, WeakPtr... etc

/**
 * @brief A (simple) semantic world representation for pick and place and other tasks.
 */
class SemanticWorld
{
public:
  /** @brief The signature for a callback on receiving table messages*/
  typedef std::function<void()> TableCallbackFn;

  /**
   * @brief A (simple) semantic world representation for pick and place and other tasks.
   * Currently this is used only to represent tables.
   */
  SemanticWorld(const rclcpp::Node::SharedPtr& node, const planning_scene::PlanningSceneConstPtr& planning_scene);

  /**
   * @brief Get all the tables within a region of interest
   */
  object_recognition_msgs::msg::TableArray getTablesInROI(double minx, double miny, double minz, double maxx,
                                                          double maxy, double maxz) const;

  /**
   * @brief Get all the tables within a region of interest
   */
  std::vector<std::string> getTableNamesInROI(double minx, double miny, double minz, double maxx, double maxy,
                                              double maxz) const;

  /**
   * @brief Generate possible place poses on the table for a given object. This chooses appropriate
   * values for min_distance_from_edge and for height_above_table based on the object properties.
   * The assumption is that the object is represented by a mesh.
   */
  std::vector<geometry_msgs::msg::PoseStamped>
  generatePlacePoses(const std::string& table_name, const shapes::ShapeConstPtr& object_shape,
                     const geometry_msgs::msg::Quaternion& object_orientation, double resolution,
                     double delta_height = 0.01, unsigned int num_heights = 2) const;

  /**
   * @brief Generate possible place poses on the table for a given object. This chooses appropriate
   * values for min_distance_from_edge and for height_above_table based on the object properties.
   * The assumption is that the object is represented by a mesh.
   */
  std::vector<geometry_msgs::msg::PoseStamped>
  generatePlacePoses(const object_recognition_msgs::msg::Table& table, const shapes::ShapeConstPtr& object_shape,
                     const geometry_msgs::msg::Quaternion& object_orientation, double resolution,
                     double delta_height = 0.01, unsigned int num_heights = 2) const;
  /**
   * @brief Generate possible place poses on the table. This samples locations in a grid on the table at
   * the given resolution (in meters) in both X and Y directions. The locations are sampled at the
   * specified height above the table (in meters) and then at subsequent additional heights (num_heights
   * times) incremented by delta_height. Locations are only accepted if they are at least min_distance_from_edge
   * meters from the edge of the table.
   */
  std::vector<geometry_msgs::msg::PoseStamped> generatePlacePoses(const object_recognition_msgs::msg::Table& table,
                                                                  double resolution, double height_above_table,
                                                                  double delta_height = 0.01,
                                                                  unsigned int num_heights = 2,
                                                                  double min_distance_from_edge = 0.10) const;

  void clear();

  bool addTablesToCollisionWorld();

  visualization_msgs::msg::MarkerArray
  getPlaceLocationsMarker(const std::vector<geometry_msgs::msg::PoseStamped>& poses) const;

  void addTableCallback(const TableCallbackFn& table_callback)
  {
    table_callback_ = table_callback;
  }

  std::string findObjectTable(const geometry_msgs::msg::Pose& pose, double min_distance_from_edge = 0.0,
                              double min_vertical_offset = 0.0) const;

  bool isInsideTableContour(const geometry_msgs::msg::Pose& pose, const object_recognition_msgs::msg::Table& table,
                            double min_distance_from_edge = 0.0, double min_vertical_offset = 0.0) const;

private:
  shapes::Mesh* createSolidMeshFromPlanarPolygon(const shapes::Mesh& polygon, double thickness) const;

  shapes::Mesh* orientPlanarPolygon(const shapes::Mesh& polygon) const;

  void tableCallback(const object_recognition_msgs::msg::TableArray::ConstSharedPtr& msg);

  void transformTableArray(object_recognition_msgs::msg::TableArray& table_array) const;

  planning_scene::PlanningSceneConstPtr planning_scene_;

  rclcpp::Node::SharedPtr node_handle_;

  object_recognition_msgs::msg::TableArray table_array_;

  std::vector<geometry_msgs::msg::PoseStamped> place_poses_;

  std::map<std::string, object_recognition_msgs::msg::Table> current_tables_in_collision_world_;

  rclcpp::Subscription<object_recognition_msgs::msg::TableArray>::SharedPtr table_subscriber_;

  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr visualization_publisher_;
  rclcpp::Publisher<moveit_msgs::msg::CollisionObject>::SharedPtr collision_object_publisher_;

  TableCallbackFn table_callback_;

  rclcpp::Publisher<moveit_msgs::msg::PlanningScene>::SharedPtr planning_scene_diff_publisher_;

  rclcpp::Logger logger_;
};
}  // namespace semantic_world
}  // namespace moveit
