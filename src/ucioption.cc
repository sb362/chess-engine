#include "ucioption.hh"

#include <charconv>

using namespace chess;
using namespace chess::uci;

Option::Option(std::string_view name, OptionType type, std::string_view help)
	: _name(name), _type(type), _help(help)
{

}

const std::string &Option::name() const
{
	return _name;
}

OptionType Option::type() const
{
	return _type;
}

const std::string &Option::help() const
{
	return _help;
}

void Option::reset()
{
	set_value(default_value_as_string());
}

ComboOption::ComboOption(std::string_view name, std::string_view default_value,
						std::unordered_set<std::string> choices, std::string_view help)
	: Option(name, OptionType::Combo, help), _value(default_value),
		_default_value(default_value), _choices(choices)
{

}

const std::string &ComboOption::default_value() const
{
	return _value;
}

const std::string &ComboOption::value() const
{
	return _value;
}

std::string ComboOption::default_value_as_string() const
{
	return _default_value;
}

std::string ComboOption::value_as_string() const
{
	return _value;
}

bool ComboOption::set_value(std::string_view value)
{
	// :(
	std::string key {value};
	if (_choices.find(key) != _choices.end())
	{
		_value = value;
		return true;
	}
	
	return false;
}

std::string ComboOption::to_string() const
{
	std::string s = fmt::format("option name {} type combo default {}", name(), default_value());
	for (const std::string &choice : _choices)
		s += " var " + choice;

	return s;
}

SpinOption::SpinOption(std::string_view name, int default_value,
						int min, int max, std::string_view help)
	: Option(name, OptionType::Spin, help), _value(default_value),
		_default_value(default_value), _min(min), _max(max)
{

}

int SpinOption::max() const
{
	return _max;
}

int SpinOption::min() const
{
	return _min;
}

int SpinOption::default_value() const
{
	return _default_value;
}

int SpinOption::value() const
{
	return _value;
}

std::string SpinOption::default_value_as_string() const
{
	return fmt::format("{}", default_value());
}

std::string SpinOption::value_as_string() const
{
	return fmt::format("{}", value());
}

bool SpinOption::set_value(std::string_view value)
{
	int i = default_value();
	const auto result = std::from_chars(value.data(), value.data() + value.size(), i);

	if (result.ec == std::errc::invalid_argument || result.ec == std::errc::result_out_of_range)
		return false;

	if (util::clamp(i, min(), max()) != i)
		return false;

	_value = i;
	return true;
}

std::string SpinOption::to_string() const
{
	return fmt::format("option name {} type spin default {} min {} max {}",
						name(), default_value(), min(), max());
}

CheckOption::CheckOption(std::string_view name, bool default_value, std::string_view help)
	: Option(name, OptionType::Check, help), _value(default_value), _default_value(default_value)
{

}

bool CheckOption::default_value() const
{
	return _default_value;
}

bool CheckOption::value() const
{
	return _value;
}

std::string CheckOption::default_value_as_string() const
{
	return default_value() ? "true" : "false";
}

std::string CheckOption::value_as_string() const
{
	return value() ? "true" : "false";
}

bool CheckOption::set_value(std::string_view value)
{
	_value = !(value == "false" || value == "no" || value == "0" || value == "");
	return true;
}

std::string CheckOption::to_string() const
{
	return fmt::format("option name {} type check default {}", name(), default_value_as_string());
}

StringOption::StringOption(std::string_view name, std::string_view default_value,
							std::string_view help)
	: Option(name, OptionType::String, help), _value(default_value), _default_value(default_value)
{

}

const std::string &StringOption::default_value() const
{
	return _default_value;
}

const std::string &StringOption::value() const
{
	return _value;
}

std::string StringOption::default_value_as_string() const
{
	return _default_value;
}

std::string StringOption::value_as_string() const
{
	return _value;
}

bool StringOption::set_value(std::string_view value)
{
	_value = value;
	return true;
}

std::string StringOption::to_string() const
{
	return fmt::format("option name {} type string default {}", name(), default_value());
}

Options::Options()
	: options(), callbacks()
{
}

Options::~Options()
{
	for (const auto &it : options)
		delete it.second;
}

std::string Options::to_string() const
{
	std::string s;
	for (const auto &it : options)
		s += it.second->to_string() + '\n';

	return s;
}

int Options::set(const std::string &name, const std::string &value)
{
	if (const auto &option_it = options.find(name); option_it != options.end())
	{
		Option *option = option_it->second;
		const std::string old_value = option->value_as_string();

		if (!option->set_value(value))
			return 2;

		if (const auto &callbacks_it = callbacks.find(name); callbacks_it != callbacks.end())
			for (Callback callback : callbacks_it->second)
				callback(option_it->second, old_value, value);

		return 0;
	}

	return 1;
}

void Options::listen(const std::string &name, Callback callback)
{
	ASSERT(callbacks.count(name) != 0);

	callbacks.find(name)->second.push_back(callback);
}
