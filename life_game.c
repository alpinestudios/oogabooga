typedef struct Item_Data
{
    string item_name;
    float64 some_property;
} Item_Data;

void modify_item(Item_Data *item)
{
    Item_Data new_item;
    new_item.item_name = STR("OOga Booga\n");
    new_item.some_property = 6;
    *item = new_item;
}

int entry(int argc, char **argv)
{
    window.title = STR("Hello World");
    bool is_game_running = true;

    Gfx_Font *font = load_font_from_disk(STR("C:/windows/fonts/verdana.ttf"), get_heap_allocator());
    assert(font, "Failed loading arial.ttf");

    string input = STR("y");
    // os_wait_and_read_stdin(&input, 1012, get_heap_allocator());

    Item_Data item;

    modify_item(&item);

    print("ITEM %s", &item.item_name);

    while (is_game_running && !window.should_close)
    {
        if (is_key_down(KEY_ESCAPE))
        {
            is_game_running = false;
        }

        draw_text(font, tprint("FPS: %s", input), 64, v2(-window.width / 2 + 30, window.height / 2 - 60), v2(1, 1), COLOR_RED);
        os_update();
        gfx_update();
    }
    return 0;
}