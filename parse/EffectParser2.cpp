#include "EffectParserImpl.h"

#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "ConditionParserImpl.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace {
    struct effect_parser_rules_2 {
        effect_parser_rules_2() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;

            const parse::lexer& tok =                                                       parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref =                  parse::value_ref_parser<int>();
            const parse::value_ref_parser_rule<double>::type& double_value_ref =            parse::value_ref_parser<double>();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref =       parse::value_ref_parser<std::string>();
            const parse::value_ref_parser_rule<PlanetType>::type& planet_type_value_ref =   parse::value_ref_parser<PlanetType>();
            const parse::value_ref_parser_rule<PlanetSize>::type& planet_size_value_ref =   parse::value_ref_parser<PlanetSize>();

            set_meter
                =   parse::set_non_ship_part_meter_type_enum() [ _a = _1 ] /* has some overlap with parse::set_ship_part_meter_type_enum() so can't use '>' */
                >>  parse::label(Value_token)               > double_value_ref [ _c = _1 ]
                >   (
                        (parse::label(AccountingLabel_token) > tok.string [ _val = new_<Effect::SetMeter>(_a, _c, _1) ] )
                    |    eps [ _val = new_<Effect::SetMeter>(_a, _c) ]
                    )
                ;

            set_ship_part_meter
                =    parse::set_ship_part_meter_type_enum() [ _a = _1 ]
                >>   parse::label(PartName_token)   > string_value_ref [ _b = _1 ]
                >    parse::label(Value_token)      > double_value_ref [ _val = new_<Effect::SetShipPartMeter>(_a, _b, _1) ]
                ;

            set_empire_stockpile
                =   tok.SetEmpireTradeStockpile_ [ _a = RE_TRADE ]
                >   (
                        (   parse::label(Empire_token) > int_value_ref [ _b = _1 ]
                        >   parse::label(Value_token)  > double_value_ref [ _val = new_<Effect::SetEmpireStockpile>(_b, _a, _1) ]
                        )
                        |  (parse::label(Value_token)  > double_value_ref [ _val = new_<Effect::SetEmpireStockpile>(_a, _1) ])
                    )
                ;

            set_empire_capital
                =    tok.SetEmpireCapital_
                >   (
                        (parse::label(Empire_token) > int_value_ref [ _val = new_<Effect::SetEmpireCapital>(_1) ])
                    |    eps [ _val = new_<Effect::SetEmpireCapital>() ]
                    )
                ;

            set_planet_type
                =    tok.SetPlanetType_
                >    parse::label(Type_token) > planet_type_value_ref [ _val = new_<Effect::SetPlanetType>(_1) ]
                ;

            set_planet_size
                =    tok.SetPlanetSize_
                >    parse::label(PlanetSize_token) > planet_size_value_ref [ _val = new_<Effect::SetPlanetSize>(_1) ]
                ;

            set_species
                =    tok.SetSpecies_
                >    parse::label(Name_token) > string_value_ref [ _val = new_<Effect::SetSpecies>(_1) ]
                ;

            set_owner
                =    tok.SetOwner_
                >    parse::label(Empire_token) > int_value_ref [ _val = new_<Effect::SetOwner>(_1) ]
                ;

            set_species_opinion
                =    tok.SetSpeciesOpinion_
                >    parse::label(Species_token) >   string_value_ref [ _a = _1 ]
                > (
                    (   parse::label(Empire_token) >  int_value_ref [ _c = _1 ]
                     >  parse::label(Opinion_token) > double_value_ref
                        [ _val = new_<Effect::SetSpeciesEmpireOpinion>(_a, _c, _1) ])
                   |
                    (   parse::label(Species_token) > string_value_ref [ _b = _1 ]
                    >   parse::label(Opinion_token) > double_value_ref
                        [ _val = new_<Effect::SetSpeciesSpeciesOpinion>(_a, _b, _1) ])
                   )
                ;

            set_visibility
                =    tok.SetVisibility_
                >   (
                        parse::label(Visibility_token)
                    > ( tok.Invisible_  [ _c = VIS_NO_VISIBILITY ]
                    |   tok.Basic_      [ _c = VIS_BASIC_VISIBILITY ]
                    |   tok.Partial_    [ _c = VIS_PARTIAL_VISIBILITY ]
                    |   tok.Full_       [ _c = VIS_FULL_VISIBILITY ]
                      )
                    > (
                        (   // empire id specified, optionally with an affiliation type:
                            // useful to specify a single recipient empire, or the allies
                            // or enemies of a single empire
                            (   (parse::label(Affiliation_token) > parse::enum_parser<EmpireAffiliationType>() [ _d = _1 ])
                            |    eps [ _d = AFFIL_SELF ]
                            )
                        >>  parse::label(Empire_token) > int_value_ref [ _b = _1 ]
                        )
                     |  (   // no empire id or condition specified, with or without an
                            // affiliation type: useful to specify no or all empires
                            (   (parse::label(Affiliation_token) > parse::enum_parser<EmpireAffiliationType>() [ _d = _1 ])
                            |    eps [ _d = AFFIL_ANY ]
                            )
                        )
                     )
                    >-(parse::label(Condition_token) > parse::detail::condition_parser [ _e = _1 ])
                    ) [ _val = new_<Effect::SetVisibility>(_c, _d, _b, _e) ]
                ;

            start
                %=  set_ship_part_meter
                |   set_meter
                |   set_empire_stockpile
                |   set_empire_capital
                |   set_planet_type
                |   set_planet_size
                |   set_species
                |   set_species_opinion
                |   set_owner
                |   set_visibility
                ;

            set_meter.name("SetMeter");
            set_ship_part_meter.name("SetShipPartMeter");
            set_empire_stockpile.name("SetEmpireStockpile");
            set_empire_capital.name("SetEmpireCapital");
            set_planet_type.name("SetPlanetType");
            set_planet_size.name("SetPlanetSize");
            set_species.name("SetSpecies");
            set_species_opinion.name("SetSpeciesOpinion");
            set_owner.name("SetOwner");
            set_visibility.name("SetVisibility");


#if DEBUG_EFFECT_PARSERS
            debug(set_meter);
            debug(set_ship_part_meter);
            debug(set_empire_stockpile);
            debug(set_empire_capital);
            debug(set_planet_type);
            debug(set_planet_size);
            debug(set_species);
            debug(set_species_opinion);
            debug(set_owner);
            debug(set_visibility);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                MeterType,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                std::string
            >,
            parse::skipper_type
        > set_meter_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                ResourceType,
                ValueRef::ValueRefBase<int>*,
                Visibility,
                EmpireAffiliationType,
                Condition::ConditionBase*
            >,
            parse::skipper_type
        > set_stockpile_or_vis_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Effect::EffectBase* (),
            qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<int>*
            >,
            parse::skipper_type
        > string_string_int_rule;

        set_meter_rule                      set_meter;
        set_meter_rule                      set_ship_part_meter;
        set_stockpile_or_vis_rule           set_empire_stockpile;
        parse::effect_parser_rule           set_empire_capital;
        parse::effect_parser_rule           set_planet_type;
        parse::effect_parser_rule           set_planet_size;
        parse::effect_parser_rule           set_species;
        string_string_int_rule              set_species_opinion;
        parse::effect_parser_rule           set_owner;
        set_stockpile_or_vis_rule           set_visibility;
        parse::effect_parser_rule           start;
    };
}

namespace parse { namespace detail {
    const effect_parser_rule& effect_parser_2() {
        static effect_parser_rules_2 retval;
        return retval.start;
    }
} }
