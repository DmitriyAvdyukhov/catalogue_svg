#pragma once
#include "json.h"

#include <memory>


namespace json
{
	enum  NodeVariant
	{
		IS_NULL,
		IS_ARRAY,
		IS_MAP,
		IS_BOOL,
		IS_INT,
		IS_DOUBLE,
		IS_STRING
	};

	enum  LastOperation
	{
		START,
		IS_KEY,
		IS_START_DICT,
		IS_START_ARRAY,
		FINISH
	};

	class BuildConstructor;
	class BuildContextFirst;
	class BuildContextSecond;
	class KeyContext;
	class ValueKeyContext;
	class ValueArrayContext;
	class DictContext;
	class ArrayContext;
	class Builder;

	class BuildConstructor
	{
	public:
		explicit BuildConstructor(Builder& builder);
	protected:
		Builder& builder_;
	};

	class BuildContextFirst : public BuildConstructor
	{
	public:
		explicit BuildContextFirst(Builder& builder);
		DictContext& StartDict();
		ArrayContext& StartArray();
	};

	class BuildContextSecond : public BuildConstructor
	{
	public:
		explicit BuildContextSecond(Builder& builder);
		KeyContext& Key(std::string key);
		Builder& EndDict();
	};

	class KeyContext : public BuildContextFirst
	{
	public:
		explicit KeyContext(Builder& builder);
		ValueKeyContext& Value(Node::Value value);
	};

	class ValueKeyContext : public BuildContextSecond
	{
	public:
		explicit ValueKeyContext(Builder& builder);
	};

	class ValueArrayContext : public BuildContextFirst
	{
	public:
		explicit ValueArrayContext(Builder& builder);
		ValueArrayContext& Value(Node::Value value);
		Builder& EndArray();
	};

	class DictContext : public BuildContextSecond
	{
	public:
		explicit DictContext(Builder& builder);
	};

	class ArrayContext : public ValueArrayContext
	{
	public:
		explicit ArrayContext(Builder& builder);
	};

	class Builder final : virtual public KeyContext, virtual public ValueKeyContext, virtual public DictContext, virtual public ArrayContext
	{
	public:
		Builder();

		DictContext& StartDict();

		ArrayContext& StartArray();

		Builder& EndDict();

		Builder& EndArray();

		KeyContext& Key(std::string s);

		Builder& Value(Node::Value value);

		Node Build();

		Builder& AddNode(Node node);

		void PushNode(const Node::Value& value);

		Dict GetContainerDict();

		Array GetContainerArray();

		template<class Container>
		Builder& EndingContainer(Container container);


	private:
		std::vector<int> last_operation_{ 0 };
		std::vector<std::string> key_;
		Node root_;
		std::vector<std::unique_ptr<Node>> nodes_stack_;
	};


	template<class Container>
	Builder& Builder::EndingContainer(Container container)
	{
		if (nodes_stack_.size() > 1)
		{
			nodes_stack_.pop_back();
			if (nodes_stack_.back()->IsArray())
			{
				Array& node = const_cast<Array&>(nodes_stack_.back()->AsArray());
				node.push_back(std::move(Node(container)));
			}
			else if (nodes_stack_.back()->IsDict())
			{
				Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
				dict[key_.back()] = std::move(Node(container));
				key_.pop_back();
				last_operation_.pop_back();
			}
		}
		else
		{
			last_operation_.push_back(LastOperation::FINISH);
			nodes_stack_.pop_back();
			nodes_stack_.push_back(std::move(std::make_unique<Node>(container)));
		}
		return *this;
	}
}

