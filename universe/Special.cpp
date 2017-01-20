#include "Special.h"

#include "Condition.h"
#include "Effect.h"
#include "UniverseObject.h"
#include "ValueRef.h"
#include "../parse/Parse.h"
#include "../util/OptionsDB.h"
#include "../util/Logger.h"
#include "../util/AppInterface.h"

#include <boost/filesystem/fstream.hpp>

namespace {
    class SpecialManager {
    public:
        SpecialManager() {

            try {
                parse::specials(m_specials);
            } catch (const std::exception& e) {
                ErrorLogger() << "Failed parsing specials: error: " << e.what();
                throw e;
            }

            if (GetOptionsDB().Get<bool>("verbose-logging")) {
                DebugLogger() << "Specials:";
                for (const std::map<std::string, Special*>::value_type& entry : m_specials) {
                    DebugLogger() << " ... " << entry.first;
                }
            }
        }
        ~SpecialManager() {
            for (std::map<std::string, Special*>::value_type& entry : m_specials) {
                delete entry.second;
            }
        }
        std::vector<std::string> SpecialNames() const {
            std::vector<std::string> retval;
            for (const std::map<std::string, Special*>::value_type& entry : m_specials) {
                retval.push_back(entry.first);
            }
            return retval;
        }
        const Special* GetSpecial(const std::string& name) const {
            std::map<std::string, Special*>::const_iterator it = m_specials.find(name);
            return it != m_specials.end() ? it->second : nullptr;
        }
    private:
        std::map<std::string, Special*> m_specials;
    };
    const SpecialManager& GetSpecialManager() {
        static SpecialManager special_manager;
        return special_manager;
    }
}

/////////////////////////////////////////////////
// Special                                     //
/////////////////////////////////////////////////
Special::~Special() {
    delete m_stealth;
    delete m_initial_capacity;
    delete m_location;
}

std::string Special::Description() const {
    std::stringstream result;

    result << UserString(m_description) << "\n";

    for (boost::shared_ptr<Effect::EffectsGroup> effect : m_effects) {
        const std::string& description = effect->GetDescription();

        if (!description.empty()) {
            result << "\n" << UserString(description) << "\n";
        }
    }

    return result.str();
}

void Special::Init() {
    if (m_stealth)
        m_stealth->SetTopLevelContent(m_name);
    for (boost::shared_ptr<Effect::EffectsGroup> effect : m_effects) {
        effect->SetTopLevelContent(m_name);
    }
    if (m_initial_capacity)
        m_initial_capacity->SetTopLevelContent(m_name);
    if (m_location)
        m_location->SetTopLevelContent(m_name);
}

std::string Special::Dump() const {
    std::string retval = DumpIndent() + "Special\n";
    ++g_indent;
    retval += DumpIndent() + "name = \"" + m_name + "\"\n";
    retval += DumpIndent() + "description = \"" + m_description + "\"\n";

    if (m_stealth)
        retval += DumpIndent() + "stealth = " + m_stealth->Dump() + "\n";

    retval += DumpIndent() + "spawnrate = " + boost::lexical_cast<std::string>(m_spawn_rate) + "\n"
           +  DumpIndent() + "spawnlimit = " + boost::lexical_cast<std::string>(m_spawn_limit) + "\n";

    if (m_initial_capacity) {
        retval += DumpIndent() + "initialcapacity = ";
        ++g_indent;
            retval += m_initial_capacity->Dump();
        --g_indent;
    }

    if (m_location) {
        retval += DumpIndent() + "location =\n";
        ++g_indent;
            retval += m_location->Dump();
        --g_indent;
    }

    if (m_effects.size() == 1) {
        retval += DumpIndent() + "effectsgroups =\n";
        ++g_indent;
        retval += m_effects[0]->Dump();
        --g_indent;
    } else {
        retval += DumpIndent() + "effectsgroups = [\n";
        ++g_indent;
        for (boost::shared_ptr<Effect::EffectsGroup> effect : m_effects) {
            retval += effect->Dump();
        }
        --g_indent;
        retval += DumpIndent() + "]\n";
    }
    retval += DumpIndent() + "graphic = \"" + m_graphic + "\"\n";
    --g_indent;
    return retval;
}

float Special::InitialCapacity(int object_id) const {
    if (!m_initial_capacity)
        return 0.0f;

    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(object_id);
    if (!obj)
        return 0.0f;

    ScriptingContext context(obj);

    return m_initial_capacity->Eval(context);
}

const Special* GetSpecial(const std::string& name)
{ return GetSpecialManager().GetSpecial(name); }

std::vector<std::string> SpecialNames()
{ return GetSpecialManager().SpecialNames(); }
