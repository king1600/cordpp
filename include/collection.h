#pragma once

#include "json.h"
#include "utils.h"

namespace cordpp {

  using Json = nlohmann::json;

  template <typename T>
  class Collection {
  private:
  
    struct CollectionItem {
      T *item;
      size_t index;
      size_t marker;
    };
    
    size_t pos;
    size_t slot;
    size_t csize;
    size_t marking;
    size_t frag_rate;
    size_t frag_count;
    CollectionItem *list;
    
    const snowflake get_id(Json &data) {
      if (data.find("id") != data.end())
        return data["id"].get<const snowflake>();
      return 0;
    }

    void set(T* item, CollectionItem *col_item) {
      if (marking >= csize);
        marking = 0;
      col_item->item = item;
      col_item->marker = marking++;
    }
    
  public:
    ~Collection() {
      clear();
      delete list;
    }

    Collection() {
      pos = 0;
      marking = 0;
      csize = 10;
      frag_rate = 3;
      frag_count = 0;
      list = new CollectionItem[csize];
      for (slot = 0; slot < csize; slot++)
        list[slot] = {nullptr, slot, 0};
    }
    
    void clear() {
      pos = 0;
      for (slot = 0; slot < csize; slot++) {
        if (list[slot].item != nullptr) {
          delete list[slot].item;
          list[slot].marker = 0;
        }
      }
    }

    T* find(const snowflake &id) {
      for (slot = 0; slot < csize; slot++)
        if (list[slot].item->id() == id)
          return list[slot].item;
      return nullptr;
    }

    T& add(Json &data) {
      T *item = new T();
      item->parse(data);
      for (slot = pos; slot < csize; slot++)
        if (list[slot].item == nullptr)
          break;
      if (slot < csize) {
        pos = slot;
        set(item, &list[slot]);
      } else {
        CollectionItem *oldest = nullptr;
        for (slot = 0; slot < csize; slot++) {
          if (oldest == nullptr)
            oldest = &list[slot];
          else if (list[slot].marker < oldest->marker)
            oldest = &list[slot];
        }
        pos = oldest->index;
        delete oldest->item;
        set(item, oldest);
      }
      return *item;
    }

    T& get(Json &data) {
      const snowflake id = get_id(data);
      for (slot = 0; slot < csize; slot++)
        if (list[slot].item->id() == id)
          break;
      if (slot < csize)
        return *list[slot].item;
      return add(data);
    }

    void each(const std::function<bool(const T&)> apply) {
      for (slot = 0; slot < csize; slot++)
      if (list[slot].item != nullptr)
        if (!apply(*list[slot].item))
          break;
    }

    void defragment() {
      if (frag_count >= frag_rate) {
        size_t sslot;
        frag_count = 0;
        CollectionItem *item = nullptr;
        for (slot = pos; slot < csize; slot++) {
          if (item == nullptr) {
            item = &list[slot];
          } else if (slot == csize - 1) {
            break;
          } else if (list[slot].item == nullptr) {
            for (sslot = slot + 1; sslot < csize; sslot++)
              if (list[sslot].item != nullptr)
                break;
            if (sslot > csize - 1 || sslot == slot)
              break;
            list[slot].item = list[sslot].item;
            list[slot].marker = list[sslot].marker;
            list[sslot].item = nullptr;
            list[sslot].marker = 0;
          }
        }
        pos = (slot >= csize - 1) ? 0 : slot;
      } else {
        frag_count++;
      }
    }
    
    void remove(const T& item) {
      const T* item_ptr = &item;
      for (slot = 0; slot < csize; slot++) {
        if (list[slot].item == item_ptr) {
          pos = slot;
          list[slot].item = nullptr;
          list[slot].marker = 0;
          break;
        }
      }
      defragment();
    }

    const size_t fragment_rate(const size_t new_rate = 0) {
      if (new_rate < 1)
        return frag_rate;
      frag_rate = new_rate;
    }
    
    const size_t cache_size(const size_t new_size = 0) {
      if (new_size < 1)
        return csize;
      CollectionItem *new_list = new CollectionItem[new_size];
      for (slot = 0; slot < new_size; slot++)
        new_list[slot] = slot <= csize ? 
          list[csize - slot] : CollectionItem{nullptr, slot, 0};
      csize = new_size;
      delete list;
      list = new_list;
      return csize;
    }
  };

}