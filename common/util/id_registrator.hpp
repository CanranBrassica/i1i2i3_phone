#pragma once

namespace IpPhone
{

template <class id_type>
struct IdRegistrator
{
    IdRegistrator()
        : m_current_id{id_type{0}} {}

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
