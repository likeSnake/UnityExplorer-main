# [MonoBehavior , Behavior , Component 三个类区别和继承关系](https://www.cnblogs.com/TreeOnEarth/p/18947686)

三者的继承关系
Object → Component → Behaviour → MonoBehaviour

一般来说都是继承MonoBehavior

三个类的区别

1. Component（组件基类）
   定义：UnityEngine.Component
   作用：所有附加到 GameObject 上的对象的基类
   关键特性：
   表示游戏对象的功能单元
   通过 gameObject 属性访问所属对象
   通过 transform 属性访问变换组件
   可以添加到/从游戏对象上移除

何时继承：
几乎不需要直接继承，因为 Unity 提供了更具体的子类
仅当需要创建完全自定义的组件类型时（非常罕见）

1. Behaviour（行为基类）
   定义：UnityEngine.Behaviour
   作用：可启用/禁用的组件基类
   关键特性：
   添加了 enabled 属性（true/false）
   禁用时不会执行任何操作
   本身是抽象类，不能直接实例化

何时继承：
当需要创建可以启用/禁用的组件时
需要基础组件功能但不需 MonoBehaviour 的生命周期方法
Unity 内置组件如 AudioSource、Light 等都继承自 Behaviour

1. MonoBehaviour（脚本基类）
   定义：UnityEngine.MonoBehaviour
   作用：所有 Unity 脚本的基类

关键特性：
完整的生命周期方法（Start(), Update(), OnEnable() 等）
协程支持（StartCoroutine()）
消息系统（SendMessage()）
编辑器集成（[SerializeField], [HideInInspector]）