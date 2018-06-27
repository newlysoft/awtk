﻿
#include "base/array.h"
#include "base/emitter.h"
#include "gtest/gtest.h"
#include <stdlib.h>

#include <string>
using std::string;

static ret_t on_event(void* ctx, event_t* e) {
  uint32_t* p = (uint32_t*)ctx;
  *p = *p + 1;
  (void)e;

  return RET_OK;
}

static ret_t on_remove(void* ctx, event_t* e) {
  uint32_t* p = (uint32_t*)ctx;
  *p = *p + 1;
  (void)e;

  return RET_REMOVE;
}

static ret_t on_remove_id(void* ctx, event_t* e) {
  uint32_t id = *((uint32_t*)ctx);
  emitter_t* emitter = (emitter_t*)e->target;

  emitter_off(emitter, id);

  return RET_REMOVE;
}

static ret_t on_stop(void* ctx, event_t* e) {
  uint32_t* p = (uint32_t*)ctx;
  *p = *p + 1;
  (void)e;

  return RET_STOP;
}

TEST(Emitter, basic) {
  event_t e;
  uint32_t n = 0;
  emitter_t emitter;
  emitter_init(&emitter);
  uint32_t type = 12;
  e.type = type;
  ASSERT_EQ(emitter_on(NULL, type, on_event, NULL), 0);
  ASSERT_EQ(emitter_on(&emitter, type, NULL, NULL), 0);

  ASSERT_EQ(emitter_on(&emitter, type, on_event, &n) > 0, true);
  ASSERT_EQ(emitter_dispatch(&emitter, &e), RET_OK);
  ASSERT_EQ(n, 1);

  ASSERT_EQ(emitter_off_by_func(&emitter, type, on_event, &n), RET_OK);
  ASSERT_EQ(emitter_dispatch(&emitter, &e), RET_OK);
  ASSERT_EQ(n, 1);

  n = 0;
  ASSERT_EQ(emitter_on(&emitter, type, on_event, &n) > 0, true);
  ASSERT_EQ(emitter_dispatch(&emitter, &e), RET_OK);
  ASSERT_EQ(n, 1);

  n = 0;
  ASSERT_EQ(emitter_on(&emitter, type, on_event, &n) > 0, true);
  ASSERT_EQ(emitter_dispatch(&emitter, &e), RET_OK);
  ASSERT_EQ(n, 2);

  n = 0;
  ASSERT_EQ(emitter_on(&emitter, type, on_event, &n) > 0, true);
  ASSERT_EQ(emitter_dispatch(&emitter, &e), RET_OK);
  ASSERT_EQ(n, 3);

  emitter_deinit(&emitter);
}

TEST(Emitter, off) {
  event_t e;
  uint32_t n = 0;
  uint32_t id = 0;
  uint32_t type = 12;
  emitter_t* emitter = emitter_create();

  e.type = type;

  id = emitter_on(emitter, type, on_event, &n);
  ASSERT_EQ(id > TK_INVALID_ID, true);
  ASSERT_EQ(emitter_size(emitter), 1);
  ASSERT_EQ(emitter_find(emitter, id) != NULL, true);

  ASSERT_EQ(emitter_off(emitter, id), RET_OK);
  ASSERT_EQ(emitter_size(emitter), 0);
  ASSERT_EQ(emitter_find(emitter, id) == NULL, true);

  id = emitter_on(emitter, type, on_event, &n);
  ASSERT_EQ(id > TK_INVALID_ID, true);
  ASSERT_EQ(emitter_size(emitter), 1);
  ASSERT_EQ(emitter_find(emitter, id) != NULL, true);

  id = emitter_on(emitter, type, on_event, &n);
  ASSERT_EQ(id > TK_INVALID_ID, true);
  ASSERT_EQ(emitter_size(emitter), 2);
  ASSERT_EQ(emitter_find(emitter, id) != NULL, true);

  ASSERT_EQ(emitter_off(emitter, id), RET_OK);
  ASSERT_EQ(emitter_size(emitter), 1);

  emitter_destroy(emitter);
}

TEST(Emitter, remove) {
  event_t e;
  uint32_t n = 0;
  uint32_t id = 0;
  uint32_t type = 12;
  emitter_t* emitter = emitter_create();

  e.type = type;

  id = emitter_on(emitter, type, on_remove, &n);
  id = emitter_on(emitter, type, on_remove, &n);
  id = emitter_on(emitter, type, on_remove, &n);
  id = emitter_on(emitter, type, on_remove, &n);

  n = 0;
  ASSERT_EQ(emitter_dispatch(emitter, &e), RET_OK);
  ASSERT_EQ(emitter_size(emitter), 0);
  ASSERT_EQ(n, 4);

  emitter_destroy(emitter);
}

TEST(Emitter, stop) {
  event_t e;
  uint32_t n = 0;
  uint32_t id = 0;
  uint32_t type = 12;
  emitter_t* emitter = emitter_create();

  e.type = type;

  id = emitter_on(emitter, type, on_stop, &n);
  id = emitter_on(emitter, type, on_stop, &n);
  id = emitter_on(emitter, type, on_stop, &n);
  id = emitter_on(emitter, type, on_stop, &n);

  n = 0;
  ASSERT_EQ(emitter_size(emitter), 4);
  ASSERT_EQ(emitter_dispatch(emitter, &e), RET_STOP);
  ASSERT_EQ(emitter_size(emitter), 4);
  ASSERT_EQ(n, 1);

  emitter_destroy(emitter);
}

TEST(Emitter, remove_in_func) {
  event_t e;
  uint32_t n = 0;
  uint32_t id1 = 0;
  uint32_t id2 = 0;
  uint32_t type = 12;
  emitter_t* emitter = emitter_create();

  e.type = type;
  e.target = emitter;

  id1 = emitter_on(emitter, type, on_event, &n);
  id2 = emitter_on(emitter, type, on_remove_id, &id1);
  ASSERT_EQ(emitter_dispatch(emitter, &e), RET_OK);
  ASSERT_EQ(emitter_size(emitter), 0);

  id1 = emitter_on(emitter, type, on_event, &n);
  id2 = emitter_on(emitter, type, on_remove_id, &id2);
  ASSERT_EQ(emitter_dispatch(emitter, &e), RET_OK);
  ASSERT_EQ(emitter_size(emitter), 1);

  emitter_destroy(emitter);
}
