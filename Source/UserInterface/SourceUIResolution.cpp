
#include "StdAfx.h"
#include "SourceUIResolution.h"
#include <sstream>
#include "files/files.h"

bool has_custom_resolutions = false;
UIResolution source_ui_resolution(true, 0, 0);
Vect2f source_ui_factor;
SHELL_ANCHOR shell_anchor = SHELL_ANCHOR_DEFAULT;
std::vector<UIResolution> resolutions;

int absoluteUIPosX(float x, SHELL_ANCHOR anchor) {
    if (xm::abs(x) >= 2.0f) x /= SQSH_COORD_WIDTH_SCALE;
    if (anchor == SHELL_ANCHOR_DEFAULT) anchor = shell_anchor;
    switch (anchor) {
        case SHELL_ANCHOR_DEFAULT:
        case SHELL_ANCHOR_MENU:
        case SHELL_ANCHOR_CENTER: {
            //Centered
            float c = terRenderDevice->GetSizeX() * 0.5f;
            float sx = getUIX(anchor);
            x = c + (x - 0.5f) * sx * source_ui_factor.y;
            break;
        }
        case SHELL_ANCHOR_LEFT: {
            //Left anchored
            x = x * getUIX(anchor) * source_ui_factor.y;
            break;
        }
        case SHELL_ANCHOR_RIGHT: {
            //Right anchored
            x = (1.0f - x) * getUIX(anchor) * source_ui_factor.y;
            x = terRenderDevice->GetSizeX() - x;
            break;
        }
        case SHELL_ANCHOR_SCALED: {
            //Scaled
            x *= terRenderDevice->GetSizeX();
            break;
        }
    }
    return xm::round(x);
}

int absoluteUIPosY(float y, SHELL_ANCHOR anchor) {
    return absoluteUISizeY(y, anchor);
}

int absoluteUISizeX(float x, SHELL_ANCHOR anchor) {
    if (xm::abs(x) >= 2.0f) x /= SQSH_COORD_WIDTH_SCALE;
    if (anchor == SHELL_ANCHOR_DEFAULT) anchor = shell_anchor;
    if (anchor == SHELL_ANCHOR_SCALED) {
        x *= terRenderDevice->GetSizeX();
    } else {
        x *= getUIX(anchor) * source_ui_factor.y;
    }
    return xm::round(x);
}

int absoluteUISizeY(float y, SHELL_ANCHOR) {
    if (xm::abs(y) >= 2.0f) y /= SQSH_COORD_HEIGHT_SCALE;
    y *= terRenderDevice->GetSizeY();
    return xm::round(y);
}

void initSourceUIResolution() {
    has_custom_resolutions = false;
    
    //Load the shell anchor value from INI and cmdline
    shell_anchor = SHELL_ANCHOR_DEFAULT;
    IniManager ini("Perimeter.ini", false);
    int shellAnchorVal = ini.getInt("Graphics", "UIAnchor");
    check_command_line_parameter("uianchor", shellAnchorVal);
    if (0 <= shellAnchorVal && shellAnchorVal < SHELL_ANCHOR_DEFAULT) {
        shell_anchor = static_cast<SHELL_ANCHOR>(shellAnchorVal);
    }

    //Pick hardcoded resolutions from Config.prm
    for (int i = 0; i < RESOLUTION_COUNT; ++i) {
        auto res = RESOLUTIONS[i];
        resolutions.emplace_back(UIResolution(true, res.x, res.y));
    }
    
    //Scan for extra resolutions
    for (const auto & entry : get_content_entries_directory("resource/icons/intf")) {
        std::filesystem::path entry_path = std::filesystem::u8path(entry->path_content);
        if (std::filesystem::is_directory(entry_path)) {
            std::string name = entry_path.filename().u8string();
            size_t pos = name.find('x');
            if (pos == std::string::npos) {
                continue;
            }
            int x, y;
            std::istringstream instream(name);
            instream >> x;
            instream.seekg(1, std::ios::cur);
            instream >> y;
            if (x != 0 && y != 0) {
                //Avoid duplicates
                bool exists = false;
                for (UIResolution& res : resolutions) {
                    if (x == res.x && y == res.y) {
                        exists = true;
                        if (res.legacy) {
                            has_custom_resolutions = true;
                            //Legacy resolution exists as this new resolution, set off the flag
                            //so the game uses the new folder
                            res.legacy = false;
                            break;
                        }
                    }
                }
                //Add this resolution as there is no duplicate
                if (!exists) {
                    has_custom_resolutions = true;
                    resolutions.emplace_back(UIResolution(false, x, y));
                }
            }
        }
    }
    
    //Sort it
    std::sort(resolutions.begin(), resolutions.end(), SortResolutionVectors());
}

bool setSourceUIResolution(const Vect2i& res) {
    UIResolution old = source_ui_resolution;
    source_ui_resolution.set(true, 0, 0);
    
    //Picks a custom resolution if possible as long as the screen ratio is higher/equal than candidate
    if (has_custom_resolutions) {
        for (const UIResolution& candidate: resolutions) {
            if (static_cast<float>(res.x) / static_cast<float>(res.y)
             >= static_cast<float>(candidate.x) / static_cast<float>(candidate.y)) {
                if (candidate.x >= source_ui_resolution.x || candidate.y >= source_ui_resolution.y) {
                    source_ui_resolution.set(candidate);
                }
            }
        }
    }

    //Pick the candidate that fits best for screen resolution at available UI resolutions
    if (source_ui_resolution.legacy) {
        for (const UIResolution& candidate: resolutions) {
            if (res.x >= candidate.x && res.y >= candidate.y) {
                if (candidate.x >= source_ui_resolution.x || candidate.y >= source_ui_resolution.y) {
                    source_ui_resolution.set(candidate);
                }
            }
        }
    }
    
    //If none was found use the smallest one, this shouldn't happen now that we have minimum window size
    if (source_ui_resolution.x == 0 || source_ui_resolution.y == 0) {
        fprintf(stderr, "Couldn't set UI resolution! using lowest\n");
        source_ui_resolution.set(resolutions[0]);
    }
    
    //Calculate scaling factor between internal UI resolution and real resolution
    source_ui_factor.set(
        static_cast<float>(res.x) / static_cast<float>(source_ui_resolution.x),
        static_cast<float>(res.y) / static_cast<float>(source_ui_resolution.y)
    );

    fprintf(stdout, "Resolution: %dx%d Game UI: %dx%d Menu UI: %dx%d \n",
        res.x, res.y,
        source_ui_resolution.x, source_ui_resolution.y,
        static_cast<int>(source_ui_resolution.mx), source_ui_resolution.y
    );

    return old == source_ui_resolution;
}

const UIResolution& getMinimumUIResolution() {
    return resolutions[0];
}

const std::vector<UIResolution>& getSourceUIResolutions() {
    return resolutions;
}
