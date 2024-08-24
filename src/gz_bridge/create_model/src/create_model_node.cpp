#include "create_model_node.hpp"

CreateModelNode::CreateModelNode() : Node("create_model_node")
{
    RCLCPP_INFO(this->get_logger(), "Create Model Node started.");
    declareParameters();
    findSimulationPath();
    init();
    printDisplay();
}

void CreateModelNode::init()
{
    gz::msgs::EntityFactory req{};
    req.set_sdf_filename(target_path.string() + "/model.sdf");
    req.set_name(model_name);
    req.set_allow_renaming(false);

    auto pose = req.mutable_pose();
    pose->mutable_position()->set_x(model_pose[X]);
    pose->mutable_position()->set_y(model_pose[Y]);
    pose->mutable_position()->set_z(model_pose[Z]);
    pose->mutable_orientation()->set_w(model_quaternion[W]);
    pose->mutable_orientation()->set_x(model_quaternion[X]);
    pose->mutable_orientation()->set_y(model_quaternion[Y]);
    pose->mutable_orientation()->set_z(model_quaternion[Z]);

    gz::msgs::Boolean rep;
    std::string create_service = "/world/" + world_name + "/create";
    
    bool result;
    bool gz_called = true;

    while (gz_called)
    {
        if (_node.Request(create_service, req, 1000, rep, result))
        {
            if (!rep.data() || !result)
            {
                std::cout << "EntityFactory service call failed\n";
                return;
            }
            else
            {
                gz_called = false;
            }
        }
        else
        {
            std::cout << "Service call timed out as Gazebo has not been detected.\n";
            usleep(2000000);
        }
    }
}

void CreateModelNode::findSimulationPath()
{
    for (int i = 0; i < 5; ++i)
    {
        file_path = file_path.parent_path();
    }
    target_path = file_path / "Tools" / "simulation" / "models" / model_name;
}

void CreateModelNode::declareParameters()
{
    this->declare_parameter<std::string>("model_name", "marble_husky");
    this->declare_parameter<std::string>("world_name", "default");
    this->declare_parameter("model_pose", std::vector<double>(3, 0.0));
    this->declare_parameter("model_orientation", std::vector<double>(3, 0.0));

    model_pose = this->get_parameter("model_pose").as_double_array();
    model_euler = this->get_parameter("model_orientation").as_double_array();
    model_name = this->get_parameter("model_name").as_string();
    world_name = this->get_parameter("world_name").as_string();

    double euler[] = {model_euler[ROLL], model_euler[PITCH], model_euler[YAW]};
    eulerToQuaternion(euler, model_quaternion);
}

void CreateModelNode::printDisplay() const
{   
    RCLCPP_INFO(this->get_logger(), "Eular: %.2lf, %.2lf, %.2lf", model_pose[X], model_pose[Y], model_pose[Z]);
    RCLCPP_INFO(this->get_logger(), "Quaternion: %.2lf, %.2lf, %.2lf,  %.2lf", model_quaternion[X], 
                                                    model_quaternion[Y], model_quaternion[Z], model_quaternion[W]);
    RCLCPP_INFO_STREAM(this->get_logger(), "Model Name: " << model_name);
    RCLCPP_INFO_STREAM(this->get_logger(), "World Name: " << world_name);
    RCLCPP_INFO_STREAM(this->get_logger(), "Model Path: " << target_path.string());
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    CreateModelNode create_model;
    rclcpp::shutdown();
}