#include <gtest/gtest.h>
#include "observable.hpp"
#include <functional>

class CbClass{
public:
    CbClass(): _called(0) {}
    void operator ()(uintptr_t ptr){ _called += 1; }
    uint called() const { return _called; }
private:
    uint _called;
};

TEST(Observer, Field){
    
    CbClass cb;
    std::function<void(uintptr_t)> cbf = std::bind(&CbClass::operator (), &cb, std::placeholders::_1 );
    ObservableField<uint> field(1);
    field.setObserver(cbf);

    field = 2;
    ASSERT_EQ(field.value(),2);
    ASSERT_EQ(cb.called(),1);

    field = 3;
    ASSERT_EQ(field.value(),3);
    ASSERT_EQ(cb.called(),2);
}

TEST(Observer, Array){
    CbClass cb;
    std::function<void(uintptr_t)> cbf = std::bind(&CbClass::operator (), &cb, std::placeholders::_1 );
    ObservableField<uint[2]> field_array;
    field_array.setObserver(cbf);

    uint tab2[2] = { 2,3 };
    field_array = tab2;
    ASSERT_EQ(field_array.value()[0], 2);
    ASSERT_EQ(field_array.value()[1], 3);

    ASSERT_EQ(cb.called(),1);

    tab2[0] += 1,  tab2[1] += 1;
    field_array = tab2;
    ASSERT_EQ(field_array.value()[0], 3);
    ASSERT_EQ(field_array.value()[1], 4);
    ASSERT_EQ(cb.called(),2);

    field_array = tab2;
    ASSERT_EQ(cb.called(),2);
}