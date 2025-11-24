#pragma once
struct Scene {
  virtual ~Scene() {}
  virtual void on_key(int key) = 0;
  virtual void render() = 0;
  virtual const char *title() const = 0;
};

Scene *make_menu_scene();
Scene *make_avl_scene();
Scene *make_rbt_scene();
Scene *make_bst_scene();
Scene *make_maxheap_scene();
Scene *make_minheap_scene();
Scene *make_binomial_scene();
