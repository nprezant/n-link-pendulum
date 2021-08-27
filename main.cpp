#include <iostream>
#include <vector>
#include <cmath>

struct Pin
{
    double x;
    double y;

    Pin(double x, double y) : x(x), y(y) {}
};

struct State
{
    double x; // X position
    double y; // Y position

    double xdot; // X velocity
    double ydot; // Y velocity

    State(double x, double y, double xdot, double ydot)
        : x(x), y(y), xdot(xdot), ydot(ydot)
    {
    }

    State operator*(double k) const
    {
        return State(
                x * k,
                y * k,
                xdot * k,
                ydot * k);
    }

    State operator+(const State& other) const
    {
        return State(
                x + other.x,
                y + other.y,
                xdot + other.xdot,
                ydot + other.ydot);
    }
};

struct Node 
{
    double m; // Mass of node, kg
    double l; // Initial length to node, meter
    double k; // Spring stiffness, N/m
    double c; // Dampening, N/m/s

    State state; // State variables

    Node(double m, double l, double k, double c, const State& state)
        : m(m), l(l), k(k), c(c), state(state)
    {
    }

    Node(double m, double l, double k, double c, double x0, double y0, double xdot0, double ydot0)
        : m(m), l(l), k(k), c(c), state(x0, y0, xdot0, ydot0)
    {
    }

    Node WithState(const State& newState) const
    {
        return Node(m, l, k, c, newState);
    }
};

std::vector<State> operator*(const std::vector<State>& states, double k)
{
    auto s = std::vector<State>();
    s.reserve(states.size());

    for (const auto& state : states)
    {
        s.push_back(state * k);
    }

    return s;
}

std::vector<Node> operator+(const std::vector<Node>& nodes, const std::vector<State>& states)
{
    if (nodes.size() != states.size())
        throw std::runtime_error("Cannot add state list to node list of different sizes");

    auto s = std::vector<Node>();
    s.reserve(nodes.size());

    for (std::size_t i = 0; i < nodes.size(); i++)
    {
        s.push_back(nodes[i].WithState(nodes[i].state + states[i]));
    }

    return s;
}

std::vector<State> operator+(const std::vector<State>& statesLHS, const std::vector<State>& statesRHS)
{
    if (statesLHS.size() != statesRHS.size())
        throw std::runtime_error("Cannot add state lists of different sizes");

    auto s = std::vector<State>();
    s.reserve(statesLHS.size());

    for (std::size_t i = 0; i < statesLHS.size(); i++)
    {
        s.push_back(statesLHS[i] + statesRHS[i]);
    }

    return s;
}


std::vector<State> ComputeState(const Pin& pin, const std::vector<Node>& nodes)
{
    // Initialize return structure
    auto states = std::vector<State>();
    states.reserve(nodes.size());

    // Compute state for each node.
    // Reverse list to avoid double-computing link forces.
    // Link force (in the /next/ direction) for the last node is 0 since there is only one link connected.
    double xForceNext = 0.0;
    double yForceNext = 0.0;

    for (int n = nodes.size() - 1; n >= 0; --n)
    {
        const auto& node = nodes[n];

        // The first node is connected to the pin rather than another node
        const double xPrev = n == 0 ? pin.x : nodes[n - 1].state.x;
        const double yPrev = n == 0 ? pin.y : nodes[n - 1].state.y;

        // Distance from this node to the previous one
        const auto dist = std::sqrt(
                ( node.state.x - xPrev ) * ( node.state.x - xPrev ) +
                ( node.state.y - yPrev ) * ( node.state.y - yPrev ));
        
        // Difference in distance from initial link length
        const auto deltaS = dist - node.l;

        // Force of link on node (Hooke's Law)
        const auto force = node.k * deltaS;

        // Deconstruct force into x and y directions
        const auto xForce = force * (node.state.x - xPrev) / dist;
        const auto yForce = force * (node.state.y - yPrev) / dist;

        // Acceleration
        const auto xdotdot = 1.0 / node.m * (-xForce + xForceNext);
        const auto ydotdot = 1.0 / node.m * (-yForce + yForceNext) + 9.81;

        // Save the computed x and y forces for the next node to use
        xForceNext = xForce;
        yForceNext = yForce;

        // Assign updated state variables
        states.push_back(State(
                    node.state.xdot,
                    node.state.ydot,
                    xdotdot,
                    ydotdot));
    }

    return states;
}

class Chain
{
    Pin pin;
    std::vector<Node> nodes;

    Chain(const Pin& pin, const std::vector<Node>& nodes)
        : pin(pin), nodes(nodes)
    {
    }

    public:
    enum class Layout
    {
        Line,
        LShape,
    };

    static Chain Create(int numNodes, double m, double l, double k, double c, Layout layout)
    {
        // Defaults
        const auto pin = Pin(0, 0);
        const double xdot0 = 0;
        const double ydot0 = 0;

        // Generate nodes
        auto nodes = std::vector<Node>();
        nodes.reserve(numNodes);

        for (int n = 0; n < numNodes; n++)
        {
            double x0;
            double y0;

            switch (layout)
            {
                case Layout::Line:
                    x0 = pin.x + l * (n + 1);
                    y0 = pin.y;
                    break;

                case Layout::LShape:
                    throw std::runtime_error("Not implemented");
                default:
                    throw std::runtime_error("Invalid layout option");
            }

            const auto node = Node(m, l, k, c, x0, y0, xdot0, ydot0);
            nodes.push_back(node);
        }

        return Chain(pin, nodes);
    }

    // A second order Runga Kutta function
    void RungaKuttaStep(double deltaT)
    {
        std::cout << "Computing step function" << std::endl;
        
        const auto z1 = nodes;
        const auto f1 = ComputeState(pin, z1);

        const auto deltaZ1 = f1 * deltaT;

        const auto z2 = z1 + deltaZ1;
        const auto f2 = ComputeState(pin, z2);

        // Update node states
        for (std::size_t n = 0; n < nodes.size(); n++)
        {
            auto& node = nodes[n];
            
            // Function evaluation at specific node
            const auto& f1n = f1[n];
            const auto& f2n = f2[n];

            node.state.x += 0.5 * deltaT * (f1n.x + f2n.x);
            node.state.y += 0.5 * deltaT * (f1n.y + f2n.y);
            node.state.xdot += 0.5 * deltaT * (f1n.xdot + f2n.xdot);
            node.state.ydot += 0.5 * deltaT * (f1n.ydot + f2n.ydot);
        }
    }

    void WriteState()
    {
        std::cout << "Writing chain state" << std::endl;
        int n = 0;
        for (const auto& node : nodes)
        {
            std::cout << 
                "Node " + std::to_string(n) 
                + ": x: " + std::to_string(node.state.x)
                + ", y: " + std::to_string(node.state.y)
                << std::endl;
            n++;
        }
    }
};

int main(int argc, char *argv[])
{
    std::cout << "hello\n";

    const int numLinks = 4; // Size of chain
    
    // Default node properties
    const double m = 1;
    const double l = 1;
    const double k = 1e5;
    const double c = 0;

    const double deltaT = 1.0 / 100.0 * 1.0 / std::sqrt(k / m); // Time step increment
    const double simTime = 5; // Simulation time, seconds
    const int iterations = std::lround(simTime / deltaT);

    auto chain = Chain::Create(numLinks, m, l, k, c, Chain::Layout::Line);

    // Write initial node state
    chain.WriteState();

    for (int i = 0; i < iterations; i++)
    {
        // Increment time
        chain.RungaKuttaStep(deltaT);

        // Write node state
        chain.WriteState();
    }

    return 0;
}
