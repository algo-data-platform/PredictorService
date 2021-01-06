
struct Service {
	std::string ip;
	int port;
	std::string name;	
};


/*
 * 服务注册接口
 *
 * 服务注册分为注册列表、可用列表，可用列表主要是用来做服务的完美启动、停机设计
 */
class RegistryServiceIf {
	// 向注册中心注册	
	virtual void register(Service service) = 0;

	// 从注册中心摘除服务	
	virtual void unregister(Service service) = 0;

	// 变更服务状态为可用，客户端可调用的服务
	virtual void available(Service service) = 0;

	// 将服务变为不可用状态，但是心跳探测还会继续
	virtual void unavailable(Service service) = 0;

	// 返回所有注册的服务列表
	virtual std::vector<Service> getRegisteredServices() = 0;
};
