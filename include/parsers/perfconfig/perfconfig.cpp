#include <parsers/perfconfig/perfconfig.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

typedef parsers::perfconfig::perf_rule perf_rule;
typedef parsers::perfconfig::perf_option perf_option;

BOOST_FUSION_ADAPT_STRUCT(
	perf_option,
	(std::string, key)
	(std::string, value)
)

BOOST_FUSION_ADAPT_STRUCT(
	perf_rule,
	(std::string, name)
	(std::vector<perf_option>, options)
)

struct spirit_perfconfig_parser {

	template<class Iterator>
	bool parse_raw(Iterator first, Iterator last, parsers::perfconfig::result_type& v) {
		using qi::lexeme;
		using qi::_1;
		using qi::_2;
		using qi::_val;
		using phoenix::at_c;

		qi::rule<Iterator, std::vector<perf_rule>(), ascii::space_type> rules;
		qi::rule<Iterator, perf_rule(), ascii::space_type> rule;
		qi::rule<Iterator, std::vector<perf_option>(), ascii::space_type> options;
		qi::rule<Iterator, perf_option(), ascii::space_type> option;
		qi::rule<Iterator, std::string(), ascii::space_type> op_key;
		qi::rule<Iterator, std::string(), ascii::space_type> op_value;
		qi::rule<Iterator, std::string(), ascii::space_type> keyword;

 		rules		%= *rule;
 		rule		%= keyword >> "(" >> options >> ")";
 		options		= *(option >> ";") >> option;
 		option		= op_key						[at_c<0>(_val) = _1]
						>> ":"  >> op_value			[at_c<1>(_val) = _1]
					| op_key						[at_c<0>(_val) = _1];
		op_key		%= lexeme[+(qi::char_("-_a-zA-Z0-9*+")) >> *(qi::hold[+(qi::char_(' ')) >> +(qi::char_("-_a-zA-Z0-9+"))])];
		op_value	%= lexeme[+(qi::char_("-_a-zA-Z0-9*+")) >> *(qi::hold[+(qi::char_(' ')) >> +(qi::char_("-_a-zA-Z0-9+"))])];
		keyword		%= lexeme[+(qi::char_("-_a-zA-Z0-9*+")) >> *(qi::hold[+(qi::char_(' ')) >> +(qi::char_("-_a-zA-Z0-9+"))])];;

		return qi::phrase_parse(first, last, rules, ascii::space, v);
	}
};

bool parsers::perfconfig::parse(const std::string &str, result_type& v) {
	spirit_perfconfig_parser parser;
	return parser.parse_raw(str.begin(), str.end(), v);
}
