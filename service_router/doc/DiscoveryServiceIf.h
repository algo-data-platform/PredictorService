
class NotifyListenerIf {
	virtual void notify(Service service, List<Service> serviceList) = 0;
};

/*
 * 服务发现接口
 */
class DiscoveryServiceIf {
	// 订阅服务	
	virtual void subscribe(Service service, NotifyListenerIf listener) = 0;

	// 退订服务	
	virtual void unsubscribe(Service service) = 0;

	// 通过 service name 查询出所有的服务列表
	virtual std::vector<Service> discover(Service service) = 0;
};
