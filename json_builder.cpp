#include "json_builder.h"

using namespace json;

using namespace std::literals;

BuildConstructor::BuildConstructor(Builder& builder) : builder_(builder) {}

BuildContextFirst::BuildContextFirst(Builder& builder) : BuildConstructor(builder) {}

DictContext& BuildContextFirst::StartDict()
{
	return builder_.StartDict();
}

ArrayContext& BuildContextFirst::StartArray()
{
	return builder_.StartArray();
}

BuildContextSecond::BuildContextSecond(Builder& builder) : BuildConstructor(builder) {}

KeyContext& BuildContextSecond::Key(std::string key)
{
	return builder_.Key(key);
}

Builder& BuildContextSecond::EndDict()
{
	return builder_.EndDict();
}

KeyContext::KeyContext(Builder& builder) : BuildContextFirst(builder) {}

ValueKeyContext& KeyContext::Value(Node::Value value)
{
	return builder_.Value(value);
}

ValueKeyContext::ValueKeyContext(Builder& builder) : BuildContextSecond(builder) {}


ValueArrayContext::ValueArrayContext(Builder& builder) : BuildContextFirst(builder) {}


ValueArrayContext& ValueArrayContext::Value(Node::Value value)
{
	return builder_.Value(value);
}

Builder& ValueArrayContext::EndArray()
{
	return builder_.EndArray();
}

DictContext::DictContext(Builder& builder) : BuildContextSecond(builder) {}

ArrayContext::ArrayContext(Builder& builder) : ValueArrayContext(builder) {}


//-----------------------------Builder-------------------

Builder::Builder()
	: KeyContext(*this)
	, ValueKeyContext(*this)
	, DictContext(*this)
	, ArrayContext(*this) {}

DictContext& Builder::StartDict()
{
	if (last_operation_.back() == LastOperation::START
		|| last_operation_.back() == LastOperation::IS_KEY
		|| last_operation_.back() == LastOperation::IS_START_ARRAY)
	{
		Dict dict;		
		nodes_stack_.push_back(std::move(std::make_unique<Node>(dict)));
	}
	else
	{
		throw std::logic_error("Start Dict is not valid");
	}
	last_operation_.push_back(LastOperation::IS_START_DICT);
	return *this;
}

ArrayContext& Builder::StartArray()
{
	if (last_operation_.back() == LastOperation::START
		|| last_operation_.back() == LastOperation::IS_KEY
		|| last_operation_.back() == LastOperation::IS_START_ARRAY)
	{
		Array ar;		
		nodes_stack_.push_back(std::move(std::make_unique<Node>(ar)));
		last_operation_.push_back(LastOperation::IS_START_ARRAY);
	}
	else
	{
		throw std::logic_error("Start Array is not valid");
	}
	return *this;
}

Builder& Builder::EndDict()
{
	if (last_operation_.back() == LastOperation::IS_START_DICT)
	{
		EndingContainer(GetContainerDict());
	}
	else
	{
		throw std::logic_error("End Dict called not after start dict");
	}
	if (last_operation_.back() != LastOperation::FINISH)
	{
		last_operation_.pop_back();
	}
	return *this;
}


Builder& Builder::EndArray()
{
	if (last_operation_.back() == LastOperation::IS_START_ARRAY)
	{
		EndingContainer(GetContainerArray());
	}
	else
	{
		throw std::logic_error("End Array called not after start array");
	}
	if (last_operation_.back() != LastOperation::FINISH)
	{
		last_operation_.pop_back();
	}
	return *this;
}



KeyContext& Builder::Key(std::string key)
{
	if (last_operation_.back() == LastOperation::IS_START_DICT)
	{
		key_.push_back(std::move(key));
	}
	else
	{
		throw std::logic_error("Key called not after start dict");
	}
	last_operation_.push_back(LastOperation::IS_KEY);

	return *this;
}

Builder& Builder::Value(Node::Value value)
{
	if (last_operation_.back() == LastOperation::START
		|| last_operation_.back() == LastOperation::IS_START_ARRAY
		|| last_operation_.back() == LastOperation::IS_KEY)
	{
		PushNode(value);
		AddNode(*nodes_stack_.back().release());
	}
	else
	{
		throw std::logic_error("Value is not valid");
	}

	return *this;
}

Node Builder::Build()
{
	if (last_operation_.back() != LastOperation::FINISH)
	{
		throw std::logic_error("Build called before finish building the object  ");
	}
	else
	{
		root_ = std::move(Node(*nodes_stack_.back()));
	}
	return root_;
}

Builder& Builder::AddNode(Node node)
{
	nodes_stack_.pop_back();
	if (nodes_stack_.size())
	{
		if (nodes_stack_.back()->IsDict())
		{
			Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
			dict[key_.back()] = std::move(node);
			key_.pop_back();
		}
		else if (nodes_stack_.back()->IsArray())
		{
			Array& ar = const_cast<Array&>(nodes_stack_.back()->AsArray());
			ar.push_back(std::move(node));
		}
	}
	else
	{
		nodes_stack_.push_back(std::make_unique<Node>(node));
		last_operation_.push_back(LastOperation::FINISH);
	}
	if (last_operation_.back() == LastOperation::IS_KEY)
	{
		last_operation_.pop_back();
	}
	return *this;
}

void Builder::PushNode(const Node::Value& value)
{
	visit([this](auto&& val)
		{
			nodes_stack_.push_back(std::move(std::make_unique<Node>(val)));
		}, value);
}

Dict Builder::GetContainerDict()
{
	return nodes_stack_.back().release()->AsDict();
}

Array Builder::GetContainerArray()
{	
	return nodes_stack_.back().release()->AsArray();
}

