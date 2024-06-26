#pragma once

#include <map>
#include <string>
#include <vector>


const std::vector<std::string> BEAM_LINES {
    "b860", "bce", "bw2", "bx1", "bx2", "ip2", "iw2", "pkbhe", 
    "pksinq", "pktebhe", "pktm", "pktmte", "sinq", "unc"
};

const std::map<std::string, std::vector<std::string>> PROFILES {
    {"b860", {
            "MWL01", "MWL02", "MWL04", "MWL05",
            "MWL06", "MWL07", "MWL08", "MWL09",
            "MWL10", "MWP01", "MWP02", "MWP05",
            "MWP06", "MWP11", "MWP12", "MWP13",
            "MWP14", "MWP15", "MWP16", "MWP17",
            "MWP18", "MWP19", "MWP20", "MWP21",
            "MWP22", "MWP23", "MWP24", "MWP25",
            "MWP26", "MWP27", "MWP28", "MWP29",
            "MWP30", "MWP31", "MWP32", 
        }
    },
    {"bce", {
            "MHP11", "MHP12", "MBP01", "MBP03",
            "MBP04", "MBP05", "MBP06", "MBP07",
            "MBP08", "MBP09", "MBP10", "MBP11",
            "MBP12", "MBP13", "MBP14", "MCP01",
            "MCP02", "MCH1X", "MCH2Y"
        }
    },
    {"bw2", {
            "MWL01", "MWL02", "MWP01", "MWP02",
            "MWL03", "MWL04", "MWL05", "MWL06",
            "MWP05", "MWP06", "MWL07", "MWL08",
            "MWL09", "MWL10"
        }
    },
    {"bx1", {
            "MXP03", "MXP04", "MXP05", "MXP06",
            "MXP07", "MXP08", "MXP09", "MXP10",
            "MXP11", "MXP12", "MXP13", "MXP14",
            "MXP15", "MXP16", "MXP17", "MXP18",
            "MXP19", "MXP20", "MXP21", "MXP22",
            "MXP23", "MXP24", "MXP25", "MXP26"
        }
    },
    {"bx2", {
            "MXP03", "MXP04", "MXP05", "MXP06",
            "MXP27", "MXP28", "MXP29", "MXP30"
        }
    },
    {"ip2", {
            "MYP03", "MYP04", "MYP05", "MYP06",
            "MYP07", "MYP08", "MYP13", "MYP14",
            "MYP15", "MYP16", "MYP17", "MYP18",
            "MYP19", "MYP20"
        }
    },
    {"iw2", {
            "MXP03", "MXP04", "MXP05", "MXP06",
            "MXP07", "MXP08", "MXP09", "MXP10",
            "MXP11", "MXP12", "MXP13", "MXP14",
            "MXP15", "MXP16", "MXP17", "MXP18",
            "MXP19", "MXP20", "MXP21", "MXP22",
            "MXP23", "MXP24", "MXP25", "MXP26",
            "MXP41", "MXP42", "MXP43", "MXP44",
            "MNP13", "MNP14", "MNP15", "MNP16",
            "MNP17", "MNP18", "MNP19", "MNP20"
        }
    },
    {"pkbhe", {
            "MHP01", "MHP02", "MHP03", "MHP04",
            "MHP05", "MHP06", "MHP07", "MHP08",
            "MHP09", "MHP10", "MHP11", "MHP12",
            "MHP13", "MHP14", "MHP15", "MHP16",
            "MHP17", "MHP18", "MHP19", "MHP20",
            "MHP21", "MHP22", "MHP23", "MHP24",
            "MHP25", "MHP26", "MHP27", "MHP28",
            "MHP29", "MHP30", "MHP31", "MHP32",
            "MHP33", "MHP34", "MHP41", "MHP42",
            "MHP35", "MHP36"
        }
    },
    {"pksinq", {
            "MHP01", "MHP02", "MHP03", "MHP04",
            "MHP05", "MHP06", "MHP07", "MHP08",
            "MHP09", "MHP10", "MHP11", "MHP12",
            "MHP13", "MHP14", "MHP15", "MHP16",
            "MHP17", "MHP18", "MHP19", "MHP20",
            "MHP21", "MHP22", "MHP23", "MHP24",
            "MHP25", "MHP26", "MHP27", "MHP28",
            "MHP29", "MHP30", "MHP31", "MHP32",
            "MHP33", "MHP34", "MHP41", "MHP42",
            "MHP43", "MHP44", "MHP45", "MHP46", 
            "MHP47", "MHP48", "MHP49", "MHP50", 
            "MHP51", "MHP52", "MHP53", "MHP54", 
            "MHP55", "MHP56", "MHP57", "MHP58",
        }
    },
    {"pktebhe", {
            "MHP41", "MHP42", "MHP35", "MHP36"
        }
    },
    {"pktm", {
            "MHP01", "MHP02", "MHP03", "MHP04",
            "MHP05", "MHP06", "MHP07", "MHP08",
            "MHP09", "MHP10", "MHP11", "MHP12",
            "MHP13", "MHP14", "MHP15", "MHP16",
            "MHP17", "MHP18", "MHP19", "MHP20",
            "MHP21", "MHP22"
        }
    },
    {"pktmte", {
            
            "MHP23", "MHP24", "MHP25", "MHP26", 
            "MHP27", "MHP28", "MHP29", "MHP30", 
            "MHP31", "MHP32", "MHP33", "MHP34"
        }
    },
    {"sinq", {
            "MHP41", "MHP42", "MHP43", "MHP44", 
            "MHP45", "MHP46", "MHP47", "MHP48", 
            "MHP49", "MHP50", "MHP51", "MHP52", 
            "MHP53", "MHP54", "MHP55", "MHP56", 
            "MHP57", "MHP58",
        }
    },
    {"ucn", {
            "MHP11", "MHP12", "MBP01", "MBP03",
            "MBP04", "MBP05", "MBP06", "MBP07", 
            "MBP08", "MBP09", "MBP10", "MBP11",
            "MBP12", "MBP13", "MBP14", "MBH1X",
            "MBH2Y", "MBH3X", "MBH4Y"
        }
    }
};
