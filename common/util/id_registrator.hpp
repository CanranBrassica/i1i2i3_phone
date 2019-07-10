#pragma once

namespace IpPhone
{

template <class id_type>
struct IdRegistrator
{
    IdRegistrator(id_type first_id = id_type{0})
        : m_current_id{first_id} {}

    id_type registerNewId()
    {
        return m_current_id++;
    }

    id_type peekNewId() const
    {
        return m_current_id;
    }

private:
    id_type m_current_id;
};

}  // namespace IpPhone
