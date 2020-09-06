#pragma once

#include "types.hh"

#include <functional>
#include <string_view>
#include <unordered_set>
#include <unordered_map>

namespace chess::uci
{

enum class OptionType
{
	Check,
	Spin,
	Combo,
	Button,
	String
};

class Option
{
private:
	std::string _name;
	OptionType _type;
	std::string _help;

public:
	Option(std::string_view name, OptionType type, std::string_view help = "");
	virtual ~Option() = default;

	const std::string &name() const;
	OptionType type() const;
	const std::string &help() const;

	void reset();

	virtual std::string default_value_as_string() const = 0;
	virtual std::string value_as_string() const = 0;
	virtual bool set_value(std::string_view value) = 0;

	virtual std::string to_string() const = 0;
};

class ComboOption : public Option
{
private:
	std::string _value, _default_value;
	std::unordered_set<std::string> _choices;

public:
	ComboOption(std::string_view name, std::string_view default_value,
				const std::unordered_set<std::string> &choices,
				std::string_view help = "");

	const std::string &default_value() const;
	const std::string &value() const;

	std::string default_value_as_string() const override;
	std::string value_as_string() const override;
	bool set_value(std::string_view value) override;

	std::string to_string() const override;
};

class SpinOption : public Option
{
private:
	int _value, _default_value, _min, _max;

public:
	SpinOption(std::string_view name, int default_value,
				int min, int max, std::string_view help = "");

	int max() const;
	int min() const;
	int default_value() const;
	int value() const;

	std::string default_value_as_string() const override;
	std::string value_as_string() const override;
	bool set_value(std::string_view value) override;

	std::string to_string() const override;
};

class CheckOption : public Option
{
private:
	bool _value, _default_value;

public:
	CheckOption(std::string_view name, bool default_value, std::string_view help = "");

	bool default_value() const;
	bool value() const;

	std::string default_value_as_string() const override;
	std::string value_as_string() const override;
	bool set_value(std::string_view value) override;

	std::string to_string() const override;
};

class StringOption : public Option
{
private:
	std::string _value, _default_value;

public:
	StringOption(std::string_view name, std::string_view default_value, std::string_view help = "");

	const std::string &default_value() const;
	const std::string &value() const;

	std::string default_value_as_string() const override;
	std::string value_as_string() const override;
	bool set_value(std::string_view value) override;

	std::string to_string() const override;
};

using Callback = std::function<void (const Option *, const std::string &, const std::string &)>;

class Options
{
private:
	std::unordered_map<std::string, Option *> options;
	std::unordered_map<std::string, std::vector<Callback>> callbacks;

public:
	Options();
	~Options();

	int set(const std::string &name, const std::string &value);
	void listen(const std::string &name, Callback callback);

	template <class O, typename ...Args>
	bool add(const std::string &name, Args &&...args)
	{
		if (options.find(name) == options.end())
		{
			O *option = new O(name, std::forward<Args>(args)...);
			options[name] = static_cast<Option *>(option);
			callbacks[name] = {};
			return true;
		}

		return false;
	}

	template <class O>
	O *get(const std::string &name)
	{
		if (const auto &it = options.find(name); it != options.end())
			return static_cast<O *>(it->second);

		return nullptr;
	}

	std::string to_string() const;
};

} // chess::uci
