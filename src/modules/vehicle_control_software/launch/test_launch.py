import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess, IncludeLaunchDescription, SetEnvironmentVariable
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from os.path import join as Path

# The program is ensured to terminate when the simulation is closed
from launch.actions import RegisterEventHandler, EmitEvent
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown

gz_sim_resource_path = os.environ.get("GZ_SIM_RESOURCE_PATH")

if gz_sim_resource_path:
    paths = gz_sim_resource_path.split(":")
    desired_path = None

    for path in paths:
        if "Vehicle-Control-Sotfware" in path:
            desired_path = path
            break
    
    if desired_path:
        directory = desired_path.split("/")
        sim_world_path = "/".join(directory[:-1])
        sim_world_file = simulation_world_path = Path(sim_world_path, "worlds", "test.sdf")
        simulation = ExecuteProcess(cmd=["gz", "sim", "-r", sim_world_file])
    else:
        print("Vehicle-Control-Sotfware directory not found in GZ_SIM_RESOURCE_PATH.")
        simulation = ExecuteProcess(cmd=["gz", "sim"])

else:
    print("GZ_SIM_RESOURCE_PATH environment variable is not set.")
    simulation = ExecuteProcess(cmd=["gz", "sim"])

bridge_imu = Node(
    package="ros_gz_bridge",
    executable="parameter_bridge",
    arguments=["/imu@sensor_msgs/msg/Imu[gz.msgs.IMU"],
)

shutdown = RegisterEventHandler(
    event_handler=OnProcessExit(
      target_action=simulation,
      on_exit=[EmitEvent(event=Shutdown)]
    )
)

def generate_launch_description():
    return LaunchDescription([
        bridge_imu,
        simulation,
        shutdown
    ]) 