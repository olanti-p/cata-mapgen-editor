#pragma once
#ifndef CATA_SRC_COMPUTER_H
#define CATA_SRC_COMPUTER_H

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "calendar.h"
#include "computer_enums.h"
#include "coordinates.h"
#include "math_parser_diag_value.h"
#include "global_vars.h"
#include "type_id.h"

class JsonObject;
class JsonOut;
class JsonValue;
class map;
class talker;

struct computer_option {
    std::string name;
    computer_action action;
    int security;

    computer_option();
    computer_option( const std::string &N, computer_action A, int S );
    // Save to/load from saves
    void serialize( JsonOut &jout ) const;
    void deserialize( const JsonObject &jo );
    // Load from data files
    static computer_option from_json( const JsonObject &jo );
};

struct computer_failure {
    computer_failure_type type;

    computer_failure();
    explicit computer_failure( computer_failure_type t ) : type( t ) {
    }
    // Save to/load from saves
    void serialize( JsonOut &jout ) const;
    void deserialize( const JsonObject &jo );
    // Load from data files
    static computer_failure from_json( const JsonObject &jo );
};

class computer
{
    public:
        computer( const std::string &new_name, int new_security, map &here, tripoint_bub_ms new_loc );
        computer( const std::string &new_name, int new_security, tripoint_abs_ms new_loc );

        // Initialization
        void set_security( int Security );
        void add_option( const computer_option &opt );
        void add_eoc( const effect_on_condition_id &eoc );
        void add_chat_topic( const std::string &topic );
        void add_option( const std::string &opt_name, computer_action action, int security );
        void add_failure( const computer_failure &failure );
        void add_failure( computer_failure_type failure );
        void set_access_denied_msg( const std::string &new_msg );
        void set_mission( int id );
        // Save/load
        void load_legacy_data( const std::string &data );
        void serialize( JsonOut &jout ) const;
        void deserialize( const JsonValue &jv );

        friend class computer_session;
        tripoint_abs_ms loc;
        // "Jon's Computer", "Lab 6E77-B Terminal Omega"
        std::string name;
        // Linked to a mission?
        int mission_id;
        // Difficulty of simply logging in
        int security;
        // Number of times security is tripped
        int alerts;
        // Date of next attempt
        time_point next_attempt;
        // Things we can do
        std::vector<computer_option> options;
        // Things that happen if we fail a hack
        std::vector<computer_failure> failures;
        // Message displayed when the computer is secured and initial login fails.
        // Can be customized to for example warn the player of potentially lethal
        // consequences like secubots spawning.
        std::string access_denied;
        std::vector<std::string> chat_topics; // What it has to say.
        std::vector<effect_on_condition_id> eocs; // Effect on conditions to run when accessed.
        // Miscellaneous key/value pairs.
        global_variables::impl_t values;
        // Methods for setting/getting misc key/value pairs.
        void set_value( const std::string &key, diag_value value );
        template <typename... Args>
        void set_value( const std::string &key, Args... args ) {
            set_value( key, diag_value{ std::forward<Args>( args )... } );
        }
        void remove_value( const std::string &key );
        diag_value const *maybe_get_value( const std::string &key ) const;

        void remove_option( computer_action action );
};
std::unique_ptr<talker> get_talker_for( computer &me );
std::unique_ptr<talker> get_talker_for( computer *me );

#endif // CATA_SRC_COMPUTER_H
